import argparse
import subprocess
import time
import random
import shlex
import signal

RANDOM_LIMIT = 1000
SEED = 123456789
random.seed(SEED)

AMMUNITION = [
    'localhost:8080/api/v1/maps/map1',
    'localhost:8080/api/v1/maps'
]

SHOOT_COUNT = 100
COOLDOWN = 0.1


def start_server():
    parser = argparse.ArgumentParser()
    parser.add_argument('server', type=str)
    return parser.parse_args().server


def start_perf(pid):
    return f"perf record -gs -p {pid} -o perf.data"


def run(command, stdin=None, stdout=None):
    process = subprocess.Popen(shlex.split(command), stdin=stdin, stdout=stdout, stderr=subprocess.DEVNULL)
    return process


def stop(process, wait=False):
    if process.poll() is None and wait:
        process.wait()
    process.terminate()


def shoot(ammo):
    hit = run('curl ' + ammo, stdout=subprocess.DEVNULL)
    time.sleep(COOLDOWN)
    stop(hit, wait=True)


def make_shots():
    for _ in range(SHOOT_COUNT):
        ammo_number = random.randrange(RANDOM_LIMIT) % len(AMMUNITION)
        shoot(AMMUNITION[ammo_number])
    print('Shooting complete')


def make_graph():
    with open("graph.svg", mode="w") as graph_file:
        perf = run("perf script -i perf.data", stdout=subprocess.PIPE)
        stackcollapse = run("./FlameGraph/stackcollapse-perf.pl", stdin=perf.stdout, stdout=subprocess.PIPE)
        graph = run("./FlameGraph/flamegraph.pl", stdin=stackcollapse.stdout, stdout=graph_file)
        stop(perf, True)
        stop(stackcollapse, True)
        stop(graph, True)


server = run(start_server())
perf = run(start_perf(server.pid))
make_shots()
stop(server)
time.sleep(1)
make_graph()
print('Job done')
