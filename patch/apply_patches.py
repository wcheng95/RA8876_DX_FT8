from os.path import join, isfile

Import("env")

FRAMEWORK_DIR = env.PioPlatform().get_package_dir("framework-arduinoteensy")
patchflag_path = join(FRAMEWORK_DIR, ".patching-done")

# patch file only if we didn't do it before
if not isfile(patchflag_path) :
    original_file = join(FRAMEWORK_DIR, "libraries", "Audio", "control_sgtl5000.cpp")
    patch_file = join("patch", "1-framework-arduinoteensy-audio.patch")

    assert isfile(original_file) and isfile(patch_file)

    env.Execute("patch %s %s" % (original_file, patch_file))
    
    def _touch(path):
        with open(path, "w") as fp:
            fp.write("")

    env.Execute(lambda *args, **kwargs: _touch(patchflag_path))
    