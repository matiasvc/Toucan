#!/usr/bin/env bash

cd "$( dirname "${BASH_SOURCE[0]}" )" || exit

wget https://vision.in.tum.de/rgbd/dataset/freiburg3/rgbd_dataset_freiburg3_long_office_household.tgz -O - | tar xvzf -
