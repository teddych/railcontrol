#!/usr/bin/env python3
"""
RailControl screenshot tool: list generator and image generator for the popups and buttons of the web UI.

    list    Crawls the web UI and creates or updates the image list (CSV).
            Existing rows are kept unchanged (so image names, types and URLs can be edited by hand),
            empty URLs are filled in and newly found popups/tabs/buttons are appended.
            The column "documentation" is recomputed (see check).

    check   Only recomputes the column "documentation" (and empty types) by comparing the image list
            with the Documentation directory. Needs neither RailControl nor Firefox.

    images  Creates the images listed in the image list, popups optionally in all languages,
            buttons once (language independent).

Requirements:
    - RailControl must be running (list, images)
    - Firefox (list, images)
    - Python 3.8+ and Selenium 4.11+ (pip install -r tools/screenshots/requirements.txt)
      Selenium Manager downloads geckodriver automatically if it is not in PATH.
    - Works on Linux, Windows and within Cygwin (with the Windows Firefox).

Image list (semicolon separated, UTF-8):

    image;type;documentation;command;tab;url;notes

    image          base name of the image (see --name and --button-name)
    type           popup     popup of the web UI, one image per language
                   button    button of the main menu, one image for all languages
                   other     other image of the web UI (e.g. context menu), created by hand
                   external  image not coming from RailControl (e.g. screenshot of a control), created by hand
    documentation  generated, except "excluded":
                   excluded  set by hand: image is not needed in the documentation. Kept unchanged.
                             "unneeded" is accepted as well. New rows containing "askdelete" get it by default.
                   yes       image exists in the documentation
                   no        image exists in the documentation, but does not come from RailControl (type external)
                   mismatch  image does not exist in the documentation, but with another name. Detected by the hint
                             "documentation: <name>.png" in the notes or guessed by comparing the names
                   missing   image does not exist in the documentation
    command        popup command (cmd=...) or button command (button id skip_<command>)
    tab            tab name inside the popup (empty if the popup has no tabs)
    url            popup URL. Empty: filled in by "list". "manual": has to be set by hand, skipped until then.
                   Popup rows without a URL starting with "/?cmd=" are skipped by "images". Unused for buttons.
    notes          free text. For a mismatch, "[suggestion: ...]" explains how to resolve it (generated, recreated every run)

Examples:
    python3 tools/screenshots/screenshot_tool.py list
    python3 tools/screenshots/screenshot_tool.py check
    python3 tools/screenshots/screenshot_tool.py images
    python3 tools/screenshots/screenshot_tool.py images --languages all --out-dir Documentation
    python3 tools/screenshots/screenshot_tool.py images --filter '^tracks_'
    python3 tools/screenshots/screenshot_tool.py images --type button --out-dir Documentation
"""

import argparse
import csv
import os
import re
import shutil
import subprocess
import sys
import time
import urllib.request


SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
REPO_ROOT = os.path.dirname(os.path.dirname(SCRIPT_DIR))
CRAWLER_NAME = 'rc_crawler.html'
DEFAULT_LIST = os.path.join(SCRIPT_DIR, 'images.csv')
FIELDS = ['image', 'type', 'documentation', 'command', 'tab', 'url', 'notes']

TYPE_POPUP = 'popup'
TYPE_BUTTON = 'button'
TYPE_OTHER = 'other'
TYPE_EXTERNAL = 'external'
TYPES = [TYPE_POPUP, TYPE_BUTTON, TYPE_OTHER, TYPE_EXTERNAL]

DOC_YES = 'yes'
DOC_NO = 'no'
DOC_MISMATCH = 'mismatch'
DOC_MISSING = 'missing'
DOC_EXCLUDED = 'excluded'  # set by hand: image is not needed in the documentation, never recomputed
DOC_EXCLUDED_ALIASES = {DOC_EXCLUDED, 'unneeded', 'notneeded', 'not needed'}
EXCLUDED_BY_DEFAULT = re.compile(r'askdelete')  # newly added rows matching (image or command) are excluded


# ---------------------------------------------------------------------------
# Command line
# ---------------------------------------------------------------------------

def parse_arguments():
    documentation = argparse.ArgumentParser(add_help=False)
    documentation.add_argument('--list', default=DEFAULT_LIST, help='Image list (CSV). Default: tools/screenshots/images.csv')
    documentation.add_argument('--doc-dir', default=os.path.join(REPO_ROOT, 'Documentation'),
                               help='Documentation directory used for the column "documentation". Default: <repository>/Documentation')

    common = argparse.ArgumentParser(add_help=False, parents=[documentation])
    common.add_argument('--base-url', default='http://127.0.0.1:8082', help='URL of RailControl. Default: http://127.0.0.1:8082')
    common.add_argument('--html-dir', default=os.path.join(REPO_ROOT, 'html'),
                        help='html directory served by RailControl. Default: <repository>/html')
    common.add_argument('--firefox', default=None, help='Path to the Firefox binary (optional)')
    common.add_argument('--no-headless', action='store_true', help='Show the Firefox window')

    parser = argparse.ArgumentParser(description='List generator and image generator for the RailControl web UI popups and buttons.')
    subparsers = parser.add_subparsers(dest='action', required=True)

    list_parser = subparsers.add_parser('list', parents=[common], help='Create or update the image list')
    list_parser.add_argument('--depth', type=int, default=3, help='Maximum nesting depth of popups. Default: 3')

    subparsers.add_parser('check', parents=[documentation], help='Recompute the column "documentation" of the image list')

    images_parser = subparsers.add_parser('images', parents=[common], help='Create the images of the image list')
    images_parser.add_argument('--languages', default='current',
                               help='current | all | comma separated language keys or codes (e.g. 0,de). Default: current')
    images_parser.add_argument('--type', default='all', choices=['all', TYPE_POPUP, TYPE_BUTTON],
                               help='Type of the images to create. Default: all')
    images_parser.add_argument('--filter', default='', help='Regular expression matched against the image name')
    images_parser.add_argument('--out-dir', default=os.path.join(REPO_ROOT, 'screenshots'),
                               help='Output directory. Default: <repository>/screenshots. Use Documentation to update the documentation.')
    images_parser.add_argument('--name', default='{lang}/{image}_{lang}.png',
                               help='File name pattern of popups relative to --out-dir. Placeholders: {lang} {image} {command} {tab}. '
                                    'Default: {lang}/{image}_{lang}.png (same as in Documentation)')
    images_parser.add_argument('--button-name', default='{image}.png',
                               help='File name pattern of buttons relative to --out-dir. Placeholders: {image} {command}. '
                                    'Default: {image}.png (same as in Documentation)')
    images_parser.add_argument('--settle', type=int, default=700, help='Milliseconds to wait after a popup has been loaded. Default: 700')
    return parser.parse_args()


# ---------------------------------------------------------------------------
# Image list
# ---------------------------------------------------------------------------

def is_popup_url(url):
    return url.startswith('/?cmd=')


def guess_type(row):
    if is_popup_url(row['url']) or (row['url'] == 'manual'):
        return TYPE_POPUP
    if row['notes'].lower().startswith('not railcontrol'):
        return TYPE_EXTERNAL
    return TYPE_OTHER


def read_list(file_name):
    if not os.path.exists(file_name):
        return []
    with open(file_name, newline='', encoding='utf-8') as file:
        rows = []
        for row in csv.DictReader(file, delimiter=';'):
            row = {field: (row.get(field) or '').strip() for field in FIELDS}
            if not row['image']:
                continue
            if row['type'] not in TYPES:
                if row['type']:
                    print('Unknown type "{0}" of {1}, guessing the type'.format(row['type'], row['image']))
                row['type'] = guess_type(row)
            if row['documentation'].lower() in DOC_EXCLUDED_ALIASES:
                row['documentation'] = DOC_EXCLUDED
            rows.append(row)
        return rows


def write_list(file_name, rows):
    with open(file_name, 'w', newline='', encoding='utf-8') as file:
        writer = csv.DictWriter(file, fieldnames=FIELDS, delimiter=';', lineterminator='\n')
        writer.writeheader()
        writer.writerows(rows)


def default_image_name(command, tab):
    return command + '_' + tab if tab else command


def default_button_name(command):
    return 'menu_' + command


def row_key(row):
    return (row['type'], row['command'], row['tab'])


# ---------------------------------------------------------------------------
# Documentation
# ---------------------------------------------------------------------------

class Documentation:
    """Images of the documentation: <doc-dir>/<lang>/<stem>_<lang>.png (per language) and <doc-dir>/<stem>.png (buttons)."""

    def __init__(self, doc_dir):
        self.language_stems = set()
        self.root_stems = set()
        if not os.path.isdir(doc_dir):
            print('Documentation directory {0} not found, all images are missing'.format(doc_dir))
            return
        for entry in sorted(os.listdir(doc_dir)):
            path = os.path.join(doc_dir, entry)
            if os.path.isfile(path) and entry.lower().endswith('.png'):
                self.root_stems.add(entry[:-4])
                continue
            if not os.path.isdir(path):
                continue
            suffix = '_' + entry + '.png'
            for name in os.listdir(path):
                if name.endswith(suffix):
                    self.language_stems.add(name[:-len(suffix)])


TOKEN_SUFFIXES = ('askdelete', 'list', 'edit')
IGNORED_TOKENS = {'menu', 'config'}
MIN_PREFIX = 4
DOC_HINT = re.compile(r'documentation:\s*([^\s;]+?)(?:_[a-z]{2})?\.png')
# Generated suggestion in the notes, removed and recreated by every "list" and "check"
SUGGESTION = re.compile(r'\s*\[suggestion:[^\]]*\]')


def strip_suggestion(notes):
    return SUGGESTION.sub('', notes).strip()


def add_suggestion(row, stem):
    if row['type'] == TYPE_BUTTON:
        file_name = stem + '.png'
    else:
        file_name = '<lang>/' + stem + '_<lang>.png'
    text = ("[suggestion: the documentation uses {0}. Rename image '{1}' to '{2}' in this list "
            "or rename the documentation file and its references to '{1}']").format(file_name, row['image'], stem)
    row['notes'] = (row['notes'] + ' ' + text).strip()


def tokens_of(*texts):
    tokens = set()
    for text in texts:
        for token in re.split(r'[^a-z0-9]+', text.lower()):
            if (not token) or (token in IGNORED_TOKENS):
                continue
            tokens.add(token)
            for suffix in TOKEN_SUFFIXES:
                if token.endswith(suffix) and (len(token) > len(suffix)):
                    tokens.add(token[:-len(suffix)])
    return tokens


def tokens_match(a, b):
    shorter, longer = (a, b) if len(a) <= len(b) else (b, a)
    if shorter == longer:
        return True
    return (len(shorter) >= MIN_PREFIX) and longer.startswith(shorter)


def similarity(row, stem):
    """0 if the documentation image stem does not fit the row, otherwise the higher the better."""
    row_tokens = tokens_of(row['image'], row['command'], row['tab'])
    stem_tokens = tokens_of(stem)
    if not stem_tokens:
        return 0
    score = 0.0
    for stem_token in stem_tokens:
        ratios = [min(len(stem_token), len(row_token)) / max(len(stem_token), len(row_token))
                  for row_token in row_tokens if tokens_match(stem_token, row_token)]
        if not ratios:
            return 0
        score += 1 + max(ratios)
    return score


def find_mismatches(rows, candidates):
    """Assigns each documentation image not referenced by the list to at most one row (best fitting first)."""
    pairs = []
    for index, row in enumerate(rows):
        for stem in candidates:
            score = similarity(row, stem)
            if score > 0:
                pairs.append((-score, index, stem))
    pairs.sort()
    result = {}
    used = set()
    for _, index, stem in pairs:
        if (index in result) or (stem in used):
            continue
        result[index] = stem
        used.add(stem)
    return result


def update_documentation(rows, doc_dir):
    """Recomputes the column "documentation". Returns the mismatch suggestions and the images only in the documentation."""
    documentation = Documentation(doc_dir)
    suggestions = []
    unreferenced = []
    for row in rows:
        row['notes'] = strip_suggestion(row['notes'])
    # Buttons are language independent (<doc-dir>/<image>.png), all other types are per language
    groups = [
        (documentation.root_stems, [row for row in rows if row['type'] == TYPE_BUTTON]),
        (documentation.language_stems, [row for row in rows if row['type'] != TYPE_BUTTON]),
    ]
    for stems, typed_rows in groups:
        candidates = stems - {row['image'] for row in typed_rows}
        # Rows excluded by hand keep their value and take no part in the mismatch detection
        typed_rows = [row for row in typed_rows if row['documentation'] != DOC_EXCLUDED]
        not_found = [row for row in typed_rows if row['image'] not in stems]
        # Explicit hint in the notes ("documentation: <name>.png") has priority over the heuristic
        mismatches = {}
        for index, row in enumerate(not_found):
            hint = DOC_HINT.search(row['notes'])
            if hint and (hint.group(1) in stems):
                mismatches[index] = hint.group(1)
        remaining = [index for index in range(len(not_found)) if index not in mismatches]
        guessed = find_mismatches([not_found[index] for index in remaining], candidates - set(mismatches.values()))
        for position, stem in guessed.items():
            mismatches[remaining[position]] = stem
        for row in typed_rows:
            if row['image'] in stems:
                row['documentation'] = DOC_NO if row['type'] == TYPE_EXTERNAL else DOC_YES
                continue
            row['documentation'] = DOC_MISSING
        for index, stem in sorted(mismatches.items()):
            not_found[index]['documentation'] = DOC_MISMATCH
            add_suggestion(not_found[index], stem)
            suggestions.append((not_found[index], stem))
        unreferenced += sorted(candidates - set(mismatches.values()))
    return suggestions, unreferenced


def report_documentation(rows, suggestions, unreferenced):
    counts = {}
    for row in rows:
        counts[row['documentation']] = counts.get(row['documentation'], 0) + 1
    print('Documentation: ' + ', '.join('{0} {1}'.format(counts.get(state, 0), state)
                                        for state in [DOC_YES, DOC_NO, DOC_MISMATCH, DOC_MISSING, DOC_EXCLUDED]))
    for row, match in suggestions:
        print('Mismatch: {0} ({1}) is probably {2} in the documentation'.format(row['image'], row['type'], match))
    for stem in unreferenced:
        print('Only in the documentation (not in the image list): ' + stem)


# ---------------------------------------------------------------------------
# Browser
# ---------------------------------------------------------------------------

def import_selenium():
    global webdriver, By, SeleniumManager, Options, Service
    try:
        from selenium import webdriver
        from selenium.webdriver.common.by import By
        from selenium.webdriver.common.selenium_manager import SeleniumManager
        from selenium.webdriver.firefox.options import Options
        from selenium.webdriver.firefox.service import Service
    except ImportError:
        sys.exit('Selenium is missing. Install it with: pip install -r tools/screenshots/requirements.txt')


def check_railcontrol(base_url):
    try:
        with urllib.request.urlopen(base_url + '/style.css', timeout=5) as response:
            response.read()
    except Exception as error:
        sys.exit('RailControl is not reachable at {0} ({1}). Start RailControl first.'.format(base_url, error))


def cygpath(option, path):
    return subprocess.check_output(['cygpath', option, path]).decode().strip()


def create_driver(arguments):
    options = Options()
    if not arguments.no_headless:
        options.add_argument('-headless')
    # Same image size on every system, independent of the display scaling (626x626 for a popup)
    options.set_preference('layout.css.devPixelsPerPx', '1.0')
    options.set_preference('browser.aboutwelcome.enabled', False)
    options.set_preference('datareporting.policy.dataSubmissionPolicyBypassNotification', True)

    service = None
    if sys.platform == 'cygwin':
        # Under Cygwin, Selenium Manager, geckodriver and Firefox are Windows programs:
        # geckodriver has to be started with a POSIX path, Firefox has to be passed with a Windows path.
        manager_arguments = ['--browser', 'firefox']
        if arguments.firefox:
            manager_arguments += ['--browser-path', cygpath('-w', arguments.firefox)]
        paths = SeleniumManager().binary_paths(manager_arguments)
        service = Service(executable_path=cygpath('-u', paths['driver_path']))
        options.binary_location = paths['browser_path']
    elif arguments.firefox:
        options.binary_location = arguments.firefox

    driver = webdriver.Firefox(options=options, service=service)
    driver.set_window_size(1400, 1000)
    driver.set_script_timeout(30 * 60)
    return driver


class Crawler:
    """Calls the functions of screenshot_tool.html (window.rc) in the browser."""

    def __init__(self, driver, base_url):
        self.driver = driver
        driver.get(base_url + '/' + CRAWLER_NAME)
        self.call('ready')

    def call(self, function, *arguments):
        script = (
            'var done = arguments[arguments.length - 1];'
            'var args = Array.prototype.slice.call(arguments, 1, arguments.length - 1);'
            'window.rc[arguments[0]].apply(null, args).then('
            '  function(result) { done({ ok: true, result: result }); },'
            '  function(error) { done({ ok: false, error: String((error && error.message) || error) }); });'
        )
        response = self.driver.execute_async_script(script, function, *arguments)
        if not response['ok']:
            raise RuntimeError('rc.{0} failed: {1}'.format(function, response['error']))
        return response['result']

    def save_element(self, element_id, file_name):
        directory = os.path.dirname(file_name)
        if directory:
            os.makedirs(directory, exist_ok=True)
        self.driver.switch_to.frame('frame')
        try:
            png = self.driver.find_element(By.ID, element_id).screenshot_as_png
        finally:
            self.driver.switch_to.default_content()
        with open(file_name, 'wb') as file:
            file.write(png)


def with_crawler(arguments, function):
    import_selenium()
    check_railcontrol(arguments.base_url)
    crawler_target = os.path.join(arguments.html_dir, CRAWLER_NAME)
    shutil.copyfile(os.path.join(SCRIPT_DIR, CRAWLER_NAME), crawler_target)
    driver = None
    try:
        driver = create_driver(arguments)
        return function(Crawler(driver, arguments.base_url))
    finally:
        if driver is not None:
            driver.quit()
        try:
            os.remove(crawler_target)
        except OSError:
            pass


# ---------------------------------------------------------------------------
# list
# ---------------------------------------------------------------------------

def generate_list(arguments, crawler):
    discovered = [{'type': TYPE_POPUP, 'command': entry['command'], 'tab': entry['tab'], 'url': entry['url']}
                  for entry in crawler.call('discover', arguments.depth)]
    discovered += [{'type': TYPE_BUTTON, 'command': entry['command'], 'tab': '', 'url': ''}
                   for entry in crawler.call('buttons')]
    rows = read_list(arguments.list)

    discovered_by_key = {}
    for entry in discovered:
        discovered_by_key.setdefault(row_key(entry), entry)

    known_keys = set()
    filled = 0
    for row in rows:
        key = row_key(row)
        known_keys.add(key)
        if (row['type'] == TYPE_POPUP) and (row['url'] == '') and (key in discovered_by_key):
            row['url'] = discovered_by_key[key]['url']
            filled += 1

    added = 0
    for entry in discovered:
        key = row_key(entry)
        if key in known_keys:
            continue
        known_keys.add(key)
        if entry['type'] == TYPE_BUTTON:
            image = default_button_name(entry['command'])
        else:
            image = default_image_name(entry['command'], entry['tab'])
        excluded = EXCLUDED_BY_DEFAULT.search(image) or EXCLUDED_BY_DEFAULT.search(entry['command'])
        rows.append({
            'image': image,
            'type': entry['type'],
            'documentation': DOC_EXCLUDED if excluded else '',
            'command': entry['command'],
            'tab': entry['tab'],
            'url': entry['url'],
            'notes': '',
        })
        added += 1

    missing = [row for row in rows
               if (row['type'] in (TYPE_POPUP, TYPE_BUTTON)) and row['command']
               and (row_key(row) not in discovered_by_key) and (row['url'] != 'manual')]

    suggestions, unreferenced = update_documentation(rows, arguments.doc_dir)
    write_list(arguments.list, rows)
    print('{0} popup states and buttons discovered, {1} URLs filled in, {2} rows added, {3} rows in {4}'.format(
        len(discovered), filled, added, len(rows), arguments.list))
    for row in missing:
        print('Not found in the web UI: {0} (type={1}, command={2}, tab={3})'.format(row['image'], row['type'], row['command'], row['tab']))
    report_documentation(rows, suggestions, unreferenced)


# ---------------------------------------------------------------------------
# check
# ---------------------------------------------------------------------------

def check_list(arguments):
    rows = read_list(arguments.list)
    if not rows:
        sys.exit('Image list {0} is empty or does not exist. Run "list" first.'.format(arguments.list))
    suggestions, unreferenced = update_documentation(rows, arguments.doc_dir)
    write_list(arguments.list, rows)
    print('{0} rows in {1}'.format(len(rows), arguments.list))
    report_documentation(rows, suggestions, unreferenced)


# ---------------------------------------------------------------------------
# images
# ---------------------------------------------------------------------------

def select_languages(requested, available, current):
    if requested == 'current':
        return [language for language in available if language['key'] == current]
    if requested == 'all':
        return available
    selected = []
    for item in [item.strip() for item in requested.split(',') if item.strip()]:
        matches = [language for language in available if item in (language['key'], language['code'])]
        if not matches:
            sys.exit('Unknown language: {0}. Available: {1}'.format(
                item, ', '.join('{0}={1}'.format(language['key'], language['code']) for language in available)))
        selected += matches
    return selected


def output_file(out_dir, pattern, **values):
    return os.path.join(out_dir, *pattern.format(**values).split('/'))


def generate_button_images(arguments, crawler, rows, failed):
    count = 0
    try:
        for row in rows:
            file_name = output_file(arguments.out_dir, arguments.button_name, image=row['image'], command=row['command'])
            element_id = 'skip_' + row['command']
            if not crawler.call('showButton', element_id):
                failed.append(row['image'])
                print('FAILED  ' + file_name)
                continue
            crawler.save_element(element_id, file_name)
            count += 1
            print('created ' + file_name)
    finally:
        crawler.call('restoreButtons')
    return count


def generate_popup_images(arguments, crawler, rows, failed):
    info = crawler.call('languages')
    original = info['current']
    languages = select_languages(arguments.languages, info['languages'], original)

    count = 0
    current = original
    try:
        for language in languages:
            if language['key'] != current:
                crawler.call('setLanguage', language['key'])
                current = language['key']
            for row in rows:
                file_name = output_file(arguments.out_dir, arguments.name,
                                        lang=language['code'], image=row['image'], command=row['command'], tab=row['tab'])
                if not crawler.call('show', row['url'], row['tab'], arguments.settle):
                    failed.append('{0} ({1})'.format(row['image'], language['code']))
                    print('FAILED  ' + file_name)
                    continue
                crawler.save_element('popup', file_name)
                crawler.call('close')
                count += 1
                print('created ' + file_name)
    finally:
        if current != original:
            crawler.call('setLanguage', original)
    return count


def generate_images(arguments, crawler):
    rows = read_list(arguments.list)
    if arguments.filter:
        pattern = re.compile(arguments.filter)
        rows = [row for row in rows if pattern.search(row['image'])]
    popup_rows = [row for row in rows if (row['type'] == TYPE_POPUP) and is_popup_url(row['url'])]
    button_rows = [row for row in rows if (row['type'] == TYPE_BUTTON) and row['command']]
    if arguments.type == TYPE_POPUP:
        button_rows = []
    elif arguments.type == TYPE_BUTTON:
        popup_rows = []
    if (not popup_rows) and (not button_rows):
        sys.exit('No images to create. Run "list" first or check --filter and --type.')

    failed = []
    count = 0
    if button_rows:
        count += generate_button_images(arguments, crawler, button_rows, failed)
    if popup_rows:
        count += generate_popup_images(arguments, crawler, popup_rows, failed)

    print('{0} images created in {1}'.format(count, arguments.out_dir))
    if failed:
        print('{0} images failed: {1}'.format(len(failed), ', '.join(failed)))


# ---------------------------------------------------------------------------

def main():
    arguments = parse_arguments()
    start = time.time()
    if arguments.action == 'list':
        with_crawler(arguments, lambda crawler: generate_list(arguments, crawler))
    elif arguments.action == 'check':
        check_list(arguments)
    else:
        with_crawler(arguments, lambda crawler: generate_images(arguments, crawler))
    print('Duration: {0:.0f} s'.format(time.time() - start))


if __name__ == '__main__':
    main()
