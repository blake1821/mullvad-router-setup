import subprocess


def bash_get(command: str):
    try:
        result = subprocess.run(
            command,
            shell=True,
            executable='/bin/bash',
            check=True,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True
        )
        return result.stdout.strip()
    except subprocess.CalledProcessError as e:
        raise e


def bash(command: str):
    try:
        subprocess.run(
            command,
            shell=True,
            executable='/bin/bash',
            check=True
        )
    except subprocess.CalledProcessError as e:
        # Handle the exception if the command fails
        print(f"Command failed with exit code {e.returncode}")
        return False
    return True