import os
import subprocess
from pathlib import Path

import requests
from pytest import fixture


class Client:
    def __init__(self, url: str):
        self.url = url
        self.session = requests.Session()

    def cmd(self, cmd: str, **kwargs):
        params = kwargs.copy()
        params['cmd'] = cmd
        response = self.session.get(self.url, params=params)
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


@fixture
def service(tmpdir: Path, port: int = 8022):
    config = f"dbfilename = {tmpdir / 'railcontrol.sqlite'}\n" \
            f"webserverport = {port}\n"

    configfile = tmpdir / 'config.conf'
    configfile.write_text(config, 'UTF-8')

    url = f"http://localhost:{port}"

    command = [os.path.join(os.getcwd(), 'railcontrol'), '--config', str(configfile)]
    with subprocess.Popen(command, cwd=tmpdir) as proc:
        client = Client(url)

        for _ in range(10):
            if client.ping():
                yield client
                break
        else:
            raise Exception("Could not connect to service")

        proc.kill()
