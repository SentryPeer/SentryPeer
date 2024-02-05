#!/bin/bash
# SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only  */
# Copyright (c) 2021 - 2024 Gavin Henry <ghenry@sentrypeer.org> */
#
#   _____            _              _____
#  / ____|          | |            |  __ \
# | (___   ___ _ __ | |_ _ __ _   _| |__) |__  ___ _ __
#  \___ \ / _ \ '_ \| __| '__| | | |  ___/ _ \/ _ \ '__|
#  ____) |  __/ | | | |_| |  | |_| | |  |  __/  __/ |
# |_____/ \___|_| |_|\__|_|   \__, |_|   \___|\___|_|
#                              __/ |
#                             |___/
#
# cd ~/src/sentrypeer
# ./packaging/build_docker.sh v3.0.2

sudo docker build --no-cache -t sentrypeer/sentrypeer:latest .
sudo docker push sentrypeer/sentrypeer:latest
sudo docker build --no-cache -t sentrypeer/sentrypeer:"$1" .
sudo docker push sentrypeer/sentrypeer:"$1"
