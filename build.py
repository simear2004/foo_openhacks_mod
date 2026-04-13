import os
import subprocess
import shutil
import argparse
import re
import glob

# Configuration
SEVEN_ZIP_EXE = r'tools\7z.exe'
VS_WHERE_EXE = r'tools\vswhere.exe'
SOLUTION_FILE = 'foo_openhacks.sln'
PROJECT_NAME = 'foo_openhacks_mod'

def get_vs_install_dir():
    try:
        result = subprocess.run(
            [VS_WHERE_EXE, '-latest', '-products', '*', '-requires', 'Microsoft.VisualStudio.Component.VC.Tools.x86.x64', '-property', 'installationPath'],
            capture_output=True,
            text=True,
            check=True
        )
        install_dir = result.stdout.strip()
        if not install_dir:
            raise RuntimeError("Cannot find Visual Studio installation path.")
        # check existence
        vs_dev_cmd_path = os.path.join(install_dir, 'Common7', 'Tools', 'VsDevCmd.bat')
        if not os.path.exists(vs_dev_cmd_path):
            raise FileNotFoundError(f"VsDevCmd.bat not found: {vs_dev_cmd_path}")
        return install_dir
    except subprocess.CalledProcessError as e:
        raise RuntimeError(f"Failed to execute vswhere: {e.stderr}") from e
    except Exception as e:
        raise RuntimeError(f"Failed to get Visual Studio path: {str(e)}") from e

def run_vs_command(vs_install_dir, commands):
    vs_dev_cmd_path = os.path.join(vs_install_dir, 'Common7', 'Tools', 'VsDevCmd.bat')
    full_cmd = f'"{vs_dev_cmd_path}" -no_logo -arch=x64 -host_arch=x64 && ' + commands
    subprocess.run(full_cmd, shell=True, check=True)

def build_project(vs_install_dir, platform, configuration, rebuild=False):
    print(f"{'Rebuild' if rebuild else 'Building'} {platform} {configuration}...")
    target = f"{PROJECT_NAME}:rebuild" if rebuild else PROJECT_NAME
    cmd = f'msbuild {SOLUTION_FILE} -nologo -v:quiet -t:{target} /p:Configuration={configuration} /p:Platform={platform}'
    run_vs_command(vs_install_dir, cmd)

def get_output_path(platform, configuration):
    platform_dir = 'win32' if platform.lower() == 'x86' else platform
    return os.path.join('bin', platform_dir, configuration, PROJECT_NAME)

def get_version_from_header():
    version_file = os.path.join(os.getcwd(), 'src', 'hacks_version.h')
    if not os.path.exists(version_file):
        print(f"Warning: Version file not found at {version_file}, using default version")
        return "0.0.0.0"
    
    with open(version_file, 'r', encoding='utf-8') as f:
        content = f.read()
    
    match = re.search(r'#define\s+HACKS_VERSION_NUM\s+"([^"]+)"', content)
    if match:
        version = match.group(1)
        print(f"Detected version: {version}")
        return version
    else:
        print("Warning: Could not parse version from header file, using default version")
        return "0.0.0.0"

def create_component_package(temp_dir, version):
    package_name = f'{PROJECT_NAME}_v{version}_EN.fb2k-component'
    print(f'Creating component package {package_name}...')
    subprocess.run([
        SEVEN_ZIP_EXE, 'a', '-tzip', package_name, '.'
    ], cwd=temp_dir, check=True)
    print(f"Created component archive: {package_name}")
    
    src_path = os.path.join(temp_dir, package_name)
    dst_path = os.path.join(os.getcwd(), package_name)
    shutil.copy2(src_path, dst_path)
    print(f"Copied to: {dst_path}")
    
    also_copy_as = os.path.join(os.getcwd(), f'{PROJECT_NAME}_EN.fb2k-component')
    shutil.copy2(src_path, also_copy_as)
    print(f"Also copied as: {also_copy_as}")

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--config', default='Release', help='Build configuration')
    parser.add_argument('--rebuild', action='store_true', help='Use rebuild target in MSBuild')
    args = parser.parse_args()

    configuration = args.config
    rebuild = args.rebuild
    platforms = ['x86', 'x64']

    vs_install_dir = get_vs_install_dir()
    print(f'Using Visual Studio installation at: {vs_install_dir}')

    version = get_version_from_header()
	
	# Step 1: Build x86 and x64
    for platform in platforms:
        build_project(vs_install_dir, platform, configuration, rebuild=rebuild)

    # Step 2: Create versioned temp directory under public/
    public_dir = os.path.join(os.getcwd(), 'public')
    if os.path.exists(public_dir):
        shutil.rmtree(public_dir)
    os.makedirs(public_dir)
    os.makedirs(os.path.join(public_dir, 'x64'), exist_ok=True)

    # Step 3: Copy files into public directory
    for platform in platforms:
        output_dir = get_output_path(platform, configuration)
        dll_name = f'{PROJECT_NAME}.dll'
        pdb_name = f'{PROJECT_NAME}.pdb'
        dll_src = os.path.join(output_dir, dll_name)
        pdb_src = os.path.join(output_dir, pdb_name)

        if platform == 'x86':
            dll_dst = os.path.join(public_dir, dll_name)
        else:
            dll_dst = os.path.join(public_dir, 'x64', dll_name)

        shutil.copy2(dll_src, dll_dst)

    # Step 4: Create component package from version directory
    create_component_package(public_dir, version)
    
    # Step 5: Clean up
    if os.path.exists(public_dir):
        shutil.rmtree(public_dir)
    
    print('\nBuild successfully.')


if __name__ == '__main__':
    main()
