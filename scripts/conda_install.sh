#!/bin/bash
#miniconda
LINK=https://repo.anaconda.com/miniconda/Miniconda3-latest-Linux-x86_64.sh
#anaconda
#LINK=https://repo.anaconda.com/archive/Anaconda3-2023.07-1-Linux-x86_64.sh
FILE=$(basename "${LINK}")
echo ${FILE}
wget "${LINK}"
chmod +x ${FILE}
bash ./${FILE} -b -p $HOME/anaconda3/
rm "${FILE}"
eval "$($HOME/anaconda3/bin/conda shell.bash hook)"
conda init
source ~/.bashrc