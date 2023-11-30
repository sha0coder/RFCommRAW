#!/bin/bash

# Instalar dependencias del sistema
sudo apt-get update
sudo apt-get install -y libbluetooth-dev

# Instalar tu módulo de Python
python3 setup.py install

