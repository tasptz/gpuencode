from importlib.metadata import requires
from skbuild import setup

setup(
    name='gpuencode',
    author='Thomas Pönitz',
    author_email='tasptz@gmail.at',
    version='1.0.0',
    packages=['gpuencode'],
    cmake_args=['-DBUILD_PYTHON=1'],
    install_requires=['numpy']
)
