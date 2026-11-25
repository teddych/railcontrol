# Creates one object of every type per language (English, German, Spanish)
# and checks that the names (including non ASCII characters) are stored correctly.

import re

ENGLISH = 0
GERMAN = 1
SPANISH = 2
LANGUAGES = (ENGLISH, GERMAN, SPANISH)

NAMES = {
    'control':      ('Virtual control', 'Virtuelle Zentrale', 'Central virtual'),
    'loco':         ('Steam locomotive', 'Dampflok', 'Locomotora de vapor'),
    'multipleunit': ('Railcar', 'Triebzug', 'Automóvil'),
    'layer':        ('Station', 'Bahnhof', 'Estación'),
    'track':        ('Platform track', 'Bahnsteiggleis', 'Vía de andén'),
    'cluster':      ('Station tracks', 'Bahnhofsgleise', 'Vías de estación'),
    'switch':       ('Entry switch', 'Einfahrweiche', 'Desvío de entrada'),
    'signal':       ('Entry signal', 'Einfahrsignal', 'Señal de entrada'),
    'accessory':    ('Street light', 'Straßenlaterne', 'Farola'),
    'feedback':     ('Track contact', 'Gleiskontakt', 'Contacto de vía'),
    'route':        ('Route east', 'Fahrstraße Ost', 'Itinerario este'),
    'counter':      ('Counter', 'Zähler', 'Contador'),
    'text':         ('Welcome', 'Willkommen', 'Bienvenido'),
}

HARDWARE_TYPE_VIRTUAL = 1
PROTOCOL_NONE = 0
LAYER_DEFAULT = 1
ROTATION_0 = 0
TRACK_TYPE_STRAIGHT = 0
VISIBLE_NO = 0
VISIBLE_YES = 1

# Each object type gets its own row in the layout, each language its own column
ROW = {
    'track': 5,
    'switch': 7,
    'signal': 9,
    'accessory': 11,
    'feedback': 13,
    'route': 15,
    'counter': 17,
    'text': 19,
}


def text_of(response) -> str:
    return response.content.decode('utf-8')


def find_id(service, object_type: str, name: str) -> int:
    html = text_of(service.cmd(f'{object_type}list'))
    pattern = '<td>' + re.escape(name) + r'</td>(?:<td>[^<]*</td>)*<td><button id="skip_' + object_type + r'edit_(\d+)"'
    match = re.search(pattern, html)
    assert match, f'{object_type} "{name}" not found in {object_type}list'
    return int(match.group(1))


def save(service, object_type: str, language: int, **params) -> int:
    name = NAMES[object_type][language]
    params[object_type] = 0  # 0 = new object
    params['name'] = name
    response = service.cmd(f'{object_type}save', **params)
    text = text_of(response)
    # info messages start with 'i', error messages with 'e'
    assert '<html>i' in text, f'{object_type}save "{name}" failed: {text}'
    assert name in text, f'{object_type}save "{name}" returned wrong name: {text}'
    return find_id(service, object_type, name)


def column(language: int, width: int = 2) -> int:
    return language * width


# Objects that reference other objects have to be deleted first:
# routes reference tracks, multiple units reference locos, clusters reference tracks,
# all layout items are on layers and all hardware objects reference controls.
DELETE_ORDER = (
    'route',
    'multipleunit',
    'loco',
    'cluster',
    'track',
    'switch',
    'signal',
    'accessory',
    'feedback',
    'counter',
    'text',
    'layer',
    'control',
)


def delete(service, object_type: str, language: int):
    name = NAMES[object_type][language]
    object_id = find_id(service, object_type, name)
    response = service.cmd(f'{object_type}delete', **{object_type: object_id})
    text = text_of(response)
    # info messages start with 'i', error messages with 'e'
    assert '<html>i' in text, f'{object_type}delete "{name}" failed: {text}'
    assert name in text, f'{object_type}delete "{name}" returned wrong name: {text}'

    html = text_of(service.cmd(f'{object_type}list'))
    assert f'<td>{name}</td>' not in html, f'{object_type} "{name}" still in {object_type}list after delete'


def test_create_and_delete_objects_in_all_languages(service):
    controls = {}
    locos = {}
    tracks = {}

    for language in LANGUAGES:
        controls[language] = save(service, 'control', language,
            hardwaretype=HARDWARE_TYPE_VIRTUAL)

    for language in LANGUAGES:
        locos[language] = save(service, 'loco', language,
            control=controls[language], protocol=PROTOCOL_NONE, address=10 + language, length=1)

    for language in LANGUAGES:
        save(service, 'multipleunit', language,
            control=controls[language], address=20 + language, length=1,
            slavecounter=1, slave_id_1=locos[language])

    for language in LANGUAGES:
        save(service, 'layer', language)

    for language in LANGUAGES:
        tracks[language] = save(service, 'track', language,
            posx=column(language), posy=ROW['track'], posz=LAYER_DEFAULT,
            rotation=ROTATION_0, tracktype=TRACK_TYPE_STRAIGHT, length=1)

    for language in LANGUAGES:
        save(service, 'cluster', language,
            trackcounter=1, track_id_1=tracks[language])

    for language in LANGUAGES:
        save(service, 'switch', language,
            control=controls[language], protocol=PROTOCOL_NONE, address=30 + language,
            posx=column(language), posy=ROW['switch'], posz=LAYER_DEFAULT, rotation=ROTATION_0)

    for language in LANGUAGES:
        save(service, 'signal', language,
            control=controls[language], protocol=PROTOCOL_NONE, address=40 + language,
            posx=column(language), posy=ROW['signal'], posz=LAYER_DEFAULT, rotation=ROTATION_0, length=1)

    for language in LANGUAGES:
        save(service, 'accessory', language,
            control=controls[language], protocol=PROTOCOL_NONE, address=50 + language,
            posx=column(language), posy=ROW['accessory'], posz=LAYER_DEFAULT, rotation=ROTATION_0)

    for language in LANGUAGES:
        save(service, 'feedback', language,
            control=controls[language], pin=1 + language, visible=VISIBLE_YES,
            posx=column(language), posy=ROW['feedback'], posz=LAYER_DEFAULT, rotation=ROTATION_0)

    for language in LANGUAGES:
        save(service, 'route', language,
            visible=VISIBLE_NO, posx=column(language), posy=ROW['route'], posz=LAYER_DEFAULT,
            fromtrack=tracks[language], totrack=tracks[(language + 1) % len(LANGUAGES)])

    for language in LANGUAGES:
        save(service, 'counter', language,
            min=0, max=10,
            posx=column(language), posy=ROW['counter'], posz=LAYER_DEFAULT, rotation=ROTATION_0)

    for language in LANGUAGES:
        save(service, 'text', language,
            width=3,
            posx=column(language, 4), posy=ROW['text'], posz=LAYER_DEFAULT, rotation=ROTATION_0)

    # all objects must be listed with their exact names
    for object_type, names in NAMES.items():
        html = text_of(service.cmd(f'{object_type}list'))
        for name in names:
            assert f'<td>{name}</td>' in html, f'{object_type} "{name}" missing in {object_type}list'

    # delete all created objects again
    for object_type in DELETE_ORDER:
        for language in LANGUAGES:
            delete(service, object_type, language)

    # no created object must be left
    for object_type, names in NAMES.items():
        html = text_of(service.cmd(f'{object_type}list'))
        for name in names:
            assert f'<td>{name}</td>' not in html, f'{object_type} "{name}" still in {object_type}list'
