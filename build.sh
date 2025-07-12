cd Debug

dir="/opt/st/stm32cubeide_1.19.0/plugins/com.st.stm32cube.ide.mcu.externaltools.gnu-tools-for-stm32.13.3.rel1.linux64_1.0.0.202410170706/tools/bin"

if [[ ":$PATH:" != *":$dir:"* ]]; then
    export PATH="$dir:$PATH"
fi

make -j16 all