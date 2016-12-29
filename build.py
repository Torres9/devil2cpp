#!/usr/bin/env python2.7
import os
import argparse
import subprocess
from shutil import rmtree

BASEDIR = os.path.dirname(os.path.abspath(__file__))
os.chdir(BASEDIR)

with open('config.yaml', 'r') as fp:
    import yaml
    config = yaml.load(fp)
    KEYSTORE = str(config['keystore'])
    PASSWORD = str(config['password'])
    ALIAS = str(config['alias'])
    APKNAME = str(config['apk_name'])

if not APKNAME.endswith('.apk'):
    print('Error: apk name should end with ".apk"')
    exit(-1)
elif not os.path.isfile('apk/' + APKNAME):
    print('Error: apk doesn\'t exist')
    exit(-1)
else:
    APKNAME = APKNAME[:-4]

parser = argparse.ArgumentParser()
parser.add_argument("-e",
                    "--extract",
                    help="extract apk from apk directory", action="store_true")
parser.add_argument("-p",
                    "--patch",
                    help="build patched library", action="store_true")
parser.add_argument("-b",
                    "--build",
                    help="rebuild apk file", action="store_true")
args = parser.parse_args()

def extract_apk():
    subprocess.call('apktool d ' + APKNAME + '.apk', shell=True)
    os.rename(APKNAME + '/lib/armeabi-v7a/libunity.so',
        APKNAME + '/lib/armeabi-v7a/librealunity.so')

if args.extract:
    print("extract apk...")
    os.chdir('apk')

    rmtree(APKNAME, ignore_errors=True)
    extract_apk()

    os.chdir('..')
elif args.patch:
    print("patch library...")
    subprocess.call('export NDK_PROJECT_PATH=' + BASEDIR +
        '; ndk-build NDK_APPLICATION_MK=' + BASEDIR + '/Application.mk;', shell=True)
elif args.build:
    print("rebuild apk...")
    if os.path.isfile('libs/armeabi-v7a/libunity.so'):
        os.rename('libs/armeabi-v7a/libunity.so',
            'apk/' + APKNAME + '/lib/armeabi-v7a/libunity.so')

    os.chdir('apk')
    subprocess.call('apktool b ' + APKNAME, shell=True)

    os.chdir(APKNAME + '/dist')
    cmd = ('jarsigner -verbose -sigalg SHA1withRSA -digestalg SHA1 -keystore ' +
            KEYSTORE + ' -storepass ' + PASSWORD + ' ' + APKNAME + '.apk ' + ALIAS)
    subprocess.call(cmd, shell=True)

    print("install apk...")
    subprocess.call('adb install -r ' + APKNAME + '.apk', shell=True)

    os.chdir(BASEDIR)
