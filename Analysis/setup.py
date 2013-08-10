from distutils.core import setup

setup(
    name='Quality',
    version='0.1.0',
    author='Jorge Sepulveda',
    author_email='jorge_a_sepulveda@yahoo.com',
    packages=['quality', 'quality.test'],
    scripts=['bin/build-roc-table.py','bin/build-parameters-table.py'],
    url='http://pypi.python.org/pypi/Quality/',
    license='LICENSE.txt',
    description='Package to build quality metric tables.',
    long_description=open('README.txt').read(),
    install_requires=[
    "Django >= 1.1.1",
    "caldav == 0.1.4",
    ],
)
