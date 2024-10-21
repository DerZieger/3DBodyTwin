all:
	cmake -S . -B build -GNinja -DCMAKE_BUILD_TYPE=Release -Wno-dev -DCMAKE_CXX_COMPILER_LAUNCHER=ccache -DCMAKE_PREFIX_PATH="~/opensim-core;~/opensim-workspace/opensim-core-dependencies-install;~/anaconda3/envs/twin/;~/anaconda3/envs/twin/lib/python3.10/site-packages/torch/" -DDL_MODULE:BOOL=ON  && cmake --build build --parallel

debug:
	cmake -S . -B build -GNinja -DCMAKE_BUILD_TYPE=Debug -Wno-dev -DCMAKE_CXX_COMPILER_LAUNCHER=ccache -DCMAKE_PREFIX_PATH="~/opensim-core;~/opensim-workspace/opensim-core-dependencies-install;~/anaconda3/envs/twin/;~/anaconda3/envs/twin/lib/python3.10/site-packages/torch/" -DDL_MODULE:BOOL=ON  && cmake --build build --parallel

clean:
	rm -rf build external/thirdparty

setup:
	./scripts/setup.sh

setup_with_conda:
	./scripts/conda_install.sh && ./scripts/conda_setup.sh && ./scripts/setup.sh

setup_conda:
	./scripts/conda_setup.sh

install_setup_conda:
	./scripts/conda_install.sh && ./scripts/conda_setup.sh

.PHONY: all debug clean setup_conda setup setup_with_conda install_setup_conda