import os
import socket
import subprocess
from pathlib import Path

import pytest
import requests
import time


class Client:
    def __init__(self, url: str):
        self.url = url
        self.session = requests.Session()

    def cmd(self, cmd: str, **kwargs):
        params = kwargs.copy()
        params['cmd'] = cmd
        # timeout: a hanging request (e.g. deadlock) must fail the test instead of blocking forever
        response = self.session.get(self.url, params=params, timeout=10)
        response.raise_for_status()
        return response

    def ping(self):
        print('Pinging', self.url)
        try:
            response = self.session.get(self.url, timeout=10)
            response.raise_for_status()
        except requests.exceptions.ConnectionError:
            return False

        return True


SHUTDOWN_TIMEOUT = 10  # seconds to wait for a graceful shutdown
START_ATTEMPTS = 3     # number of ports tried, if RailControl can not be reached
POLL_ATTEMPTS = 10     # number of pings per start attempt
POLL_INTERVAL = 0.5    # seconds between two pings


def free_port() -> int:
    """
    Asks the operating system for a free TCP port, so that every test
    (and every pytest-xdist worker) uses its own RailControl web server port.
    The port is released again before RailControl is started, so there is a
    small chance that another process takes it in between. This is handled
    by retrying with a new port in the fixture.
    """
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as sock:
        sock.bind(('', 0))
        return sock.getsockname()[1]


def shutdown(proc: subprocess.Popen, client: Client):
    """
    Shuts RailControl down gracefully:
    1. via web server command 'shutdown'
    2. via SIGTERM
    3. via SIGKILL (last resort, test fails)
    """
    if proc.poll() is not None:
        return

    print("Requesting shutdown via web server", proc.pid)
    try:
        client.session.get(client.url, params={'cmd': 'shutdown'}, timeout=SHUTDOWN_TIMEOUT)
    except requests.exceptions.RequestException:
        # RailControl may close the connection while shutting down
        pass

    try:
        proc.wait(timeout=SHUTDOWN_TIMEOUT)
        return
    except subprocess.TimeoutExpired:
        pass

    print("Sending SIGTERM", proc.pid)
    proc.terminate()
    try:
        proc.wait(timeout=SHUTDOWN_TIMEOUT)
    except subprocess.TimeoutExpired:
        print("Sending SIGKILL", proc.pid)
        proc.kill()
        proc.wait()
    pytest.fail("RailControl did not shut down gracefully via web server")


def start(tmpdir: Path, port: int):
    """
    Starts RailControl with its own config file and database in tmpdir.
    Returns (proc, client) if the web server is reachable, otherwise (None, None).
    """
    config = "dbfilename = 'railcontrol.sqlite'\n" \
            f"webserverport = {port}\n" \
            "numkeepbackups = 0\n"

    configfile = tmpdir / 'config.conf'
    configfile.write_text(config, 'UTF-8')

    client = Client(f"http://localhost:{port}")

    command = [os.path.join(os.getcwd(), 'railcontrol'), '--config', str(configfile)]
    # railcontrol.log and railcontrol.sqlite are written to this directory
    print("Test directory", tmpdir)
    print("Starting railcontrol on port", port)
    proc = subprocess.Popen(command, cwd=tmpdir)
    print("Started", proc.pid)

    for _ in range(POLL_ATTEMPTS):
        print("Polling", proc.pid)
        if proc.poll() is not None:
            print(f"Server exited with code {proc.returncode}")
            return None, None
        if client.ping():
            return proc, client
        time.sleep(POLL_INTERVAL)

    # not reachable (e.g. port taken by another process in the meantime): stop and try again.
    # Do not use the web server command 'shutdown' here, because the port may belong
    # to another RailControl instance (e.g. of another pytest-xdist worker).
    proc.terminate()
    try:
        proc.wait(timeout=SHUTDOWN_TIMEOUT)
    except subprocess.TimeoutExpired:
        proc.kill()
        proc.wait()
    return None, None


@pytest.fixture
def service(tmpdir: Path):
    proc = None
    client = None
    for _ in range(START_ATTEMPTS):
        proc, client = start(tmpdir, free_port())
        if proc:
            break

    if not proc:
        pytest.fail("Could not connect to service")

    try:
        yield client
    finally:
        shutdown(proc, client)

    if proc.returncode != 0:
        pytest.fail(f"Server exited with code {proc.returncode}")
