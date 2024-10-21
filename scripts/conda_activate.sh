#Use for IDEs to automatically activate the environment during a routine
CONDA_ENV=twin
CONDA_PATH=~/anaconda3

if test -f "$CONDA_PATH/etc/profile.d/conda.sh"; then
    echo "Found Conda at $CONDA_PATH"
    source "$CONDA_PATH/etc/profile.d/conda.sh"
    conda --version
else
    echo "Could not find conda!"
fi

conda activate "${CONDA_ENV}"