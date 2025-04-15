import shutil, os
Import("env")
def after_upload(source, target, env):
  with open("source/firmVer.h", "r") as file:
      print("Archiving firmware.bin")
      data = file.read()
      data = data.split('#define FIRMWARE_VERSION "')[1].split('"')[0]
      print(f"version: {data}")
      if(os.path.exists("bin")):
        shutil.copy2(".pio/build/esp12/firmware.bin", f"bin/{data}.bin")
      else:
        print("bin doesn't exist")
      os.unlink(".pio/build/esp12/firmware.bin")

env.AddPostAction("buildprog", after_upload)