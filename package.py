import argparse
import subprocess
import tarfile
from os import walk, rmdir, mkdir
from os.path import isdir, isfile, join
from shutil import copytree, rmtree, copy2


def ignore_not_headers(src, names):
    def _not_header(f):
        return isfile(join(src, f)) and f[-2:] != ".h" and f[-4:] != ".hpp"

    def _dont_follow(f):
        return f in ["test", "examples", "testing", "tools",
                     "build", "buildtools"]

    return [f for f in names if _not_header(f) or _dont_follow(f)]


def remove_empty_dirs(src_dir):
    for d, _, _ in walk(src_dir, topdown=False):
        try:
            rmdir(d)
        except OSError as ex:
            pass


def copy_headers():
    print("Copying headers....")
    if isdir("include"):
        rmtree("include")
    copytree("src", "include",
             ignore=ignore_not_headers,
             ignore_dangling_symlinks=True)
    remove_empty_dirs("include")


def copy_lib(name):
    print("Copying lib %s...." % name)
    if isdir("lib"):
        rmtree("lib")
    mkdir("lib")
    copy2("src/out/Release/obj/%s" % name, "lib")


def make_tarfile(output_filename):
    print("Making tar file %s...." % output_filename)
    with tarfile.open(output_filename, "w:gz") as tar:
        tar.add("include")
        tar.add("lib")


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--os", help="Target OS (win, linux, mac, android, ios)")
    parser.add_argument("--cpu", help="Target CPU (x86, x64, arm, arm64)")
    parser.add_argument("--compiler", help="Build compiler (msvc, gcc, clang)")
    args = parser.parse_args()

    copy_headers()
    if args.compiler == "msvc":
        copy_lib("webrtc.lib")
    else:
        copy_lib("libwebrtc.a")

    git_out = subprocess.run([
        "git", "-C", "src", "rev-parse", "--short", "HEAD"],
        capture_output=True).stdout
    git_hash = git_out.decode('ascii').rstrip()
    tar_name = "webrtc-datachannel-%s-%s-%s-m73-%s.tar.gz" % (args.os, args.cpu, args.compiler, git_hash)
    make_tarfile(tar_name)


if __name__ == "__main__":
    main()
