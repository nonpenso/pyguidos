import os
from pathlib import Path

from . import _test_execution, GLOBAL_CONFIG, WORK_DIR


def configure_workspace():
    """
    Interactive CLI tool to configure the pyguidos execution workspace.
    Called via the terminal command: pyguidos-setup
    """
    print("\n" + "="*65)
    print(" pyGuidos: Execution Workspace Setup ")
    print("="*65)
    print(f"Current Workspace: {WORK_DIR}")
    print("-"*65)
    print("The GuidosToolbox engine requires a folder where it has")
    print("permissions to WRITE and EXECUTE binary files.")
    
    # Suggest a default in the user's home
    default_home = Path.home() / "pyguidos_work"
    
    print(f"\nDefault location: {default_home}")
    user_input = input("Enter a new path (or press Enter to keep current/default): ").strip()
    
    # Determine the target path based on user input
    if user_input:
        target_path = Path(user_input).resolve()
    else:
        # If they hit enter, we prioritize the existing WORK_DIR if it's already set, 
        # otherwise we suggest the new default_home.
        target_path = WORK_DIR if WORK_DIR.exists() else default_home

    print(f"\nTesting permissions in: {target_path}...")
    
    if _test_execution(target_path):
        try:
            # Save the validated path to the persistent config file
            GLOBAL_CONFIG.write_text(str(target_path), encoding="utf-8")
            print("\nSUCCESS!")
            print(f"Configuration saved to: {GLOBAL_CONFIG}")
            print(f"Your workspace is now set to: {target_path}")
            print("\nYou can now run pyguidos tools (mspa, frag, etc.) safely.")
        except Exception as e:
            print(f"\n❌ ERROR: Could not save config file: {e}")
    else:
        print("\n" + "!"*65)
        print("FAILED: Execution test failed.")
        print("!"*65)
        print(f"The directory '{target_path}' is restricted.")
        print("\nSUGGESTION:")
        if os.name == 'nt':
            print("- Try a path on a non-system drive (e.g., D:/pyguidos_work or E:/Dev).")
            print("- Ensure the folder is not marked as 'Read Only' in Windows Explorer.")
        else:
            print("- Try a directory not located in /tmp or a networked/noexec mount.")
    
    print("="*65 + "\n")

if __name__ == "__main__":
    configure_workspace()