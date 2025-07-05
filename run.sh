cd Debug

dir="/opt/st/stm32cubeide_1.17.0/plugins/com.st.stm32cube.ide.mcu.externaltools.gnu-tools-for-stm32.12.3.rel1.linux64_1.1.0.202410170702/tools/bin"

if [[ ":$PATH:" != *":$dir:"* ]]; then
    export PATH="$dir:$PATH"
fi

make all || exit 1
cd ..
openocd -f interface/stlink.cfg -f target/stm32f4x.cfg -c "program Debug/nullkart.elf verify reset exit"