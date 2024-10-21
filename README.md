3D Body Twin
=======

3D Body Twin is a framework to interactively optimize the shapemodel SUPR.  
The user can choose during runtime which constraints to use for the shape and pose optimization.  
To import marker positions our system can handle the common ez3D and osim as well as a custom format based on json.  
You will need a NVidia GPU to run this code as is, but you can probably adjust it yourself to run on CPU or AMD.

This code accompanies the paper **3D Body Twin: Improving Human Gait Visualizations Using Personalized Avatars** accepted at **[ShapeMI 2024](https://shapemi.github.io/)**


# Installation

Multiple scripts are provided in the scripts folder for installation convenience including a makefile to do it all. 
For manual installation refer to the scripts.

## Before running the program

Various dependencies for the cppgl and the required ones for this project.

### Ubuntu (22.04)
```
git clone --recurse-submodules https://github.com/DerZieger/3DBodyTwin.git
```

If you don't already have Anaconda or Miniconda use:

    make install_setup_conda 

Otherwise, to setup the environments use ( script assumes conda was installed at `~/anaconda3`, adjust if necessary):

    make setup_conda

To get all remaining necessary dependencies just use:

    make setup

### Required models
1. The vposer model v2.05 from [here](https://download.is.tue.mpg.de/download.php?domain=smplx&sfile=V02_05.zip)
2. The supr model from [here](https://supr.is.tue.mpg.de/suprbody.php)
3. Follow the instructions in `vposercpp` and `suprcpp` to convert the model files using the `twin_conv` environment.

### Building

    make

If you want to build it yourself or in an IDE make sure that the right cmake variables are set and the conda environment is activated and check if the `cmake_prefix_path` is correct for your case  

Activate the environment:

    conda activate twin

Build with cmake:

    cmake -S . -B build -Wno-dev -DCMAKE_CXX_COMPILER_LAUNCHER=ccache -DCMAKE_PREFIX_PATH="~/opensim-core;~/opensim-workspace/opensim-core-dependencies-install;~/anaconda3/envs/twin/;~/anaconda3/envs/twin/lib/python3.10/site-packages/torch/" -DDL_MODULE:BOOL=ON

# Running 

    ./viewer

You can use the GUI during runime to add marker datasets, constraints, adjust the optimizer and more.  
All keybindings can be found in the GUI under "Config".