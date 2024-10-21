#!/bin/bash

CONDA_ENV=twin_conv
CONDA_PATH=~/anaconda3

if test -f "$CONDA_PATH/etc/profile.d/conda.sh"; then
    echo "Found Conda at $CONDA_PATH"
    source "$CONDA_PATH/etc/profile.d/conda.sh"
    conda --version
else
    echo "Could not find conda!"
fi

conda update -y -n base -c defaults conda

conda create -y -n "${CONDA_ENV}" python=3.10

conda activate "${CONDA_ENV}"

pip install torch==2.4.1+cpu torchvision==0.19.1+cpu torchaudio==2.4.1+cpu --index-url https://download.pytorch.org/whl/cpu
pip install numpy==1.23.5
pip install scipy==1.14.1
pip install chumpy==0.70
pip install argparse==1.4.0
pip install PyYAML==6.0.1

CONDA_ENV=twin

conda create -y -n "${CONDA_ENV}" python=3.10

conda activate "${CONDA_ENV}"

conda install -y cuda=11.8.0 -c "nvidia/label/cuda-11.8.0" -c nvidia -c conda-forge
conda install -y cudnn=8.9.2.26 -c "nvidia/label/cuda-11.8.0" -c nvidia -c conda-forge
conda install -y ccache=4.8 -c conda-forge
conda install -y cmake=3.26.4 -c conda-forge


cd scripts
mkdir -p External/
cd External/


wget https://download.pytorch.org/libtorch/cu118/libtorch-cxx11-abi-shared-with-deps-2.0.1%2Bcu118.zip -O  libtorch.zip
unzip libtorch.zip -d .


TORCH_TARGET=$CONDA_PATH/envs/${CONDA_ENV}/lib/python3.10/site-packages/torch/
mkdir "$TORCH_TARGET"

cp -rv libtorch/* "$TORCH_TARGET"

cd ..

rm -rf External