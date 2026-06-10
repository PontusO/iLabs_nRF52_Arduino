import os

env = DefaultEnvironment()

# An extra script is required to properly handle extra link flags set in
# the `library.properties` file
if env.BoardConfig().get("build.mcu", "").startswith("nrf5284"):
    env.Append(
        LIBPATH=[os.path.realpath("src/cortex-m4/fpv4-sp-d16-hard")],
        LIBS=["nfc_t2t"]
    )
