# Stardew Valley Animation Canceling Tool For MacOS

This tool is designed to assist players of Stardew Valley in canceling animations. It listens to a selected key (Space button by default) and, when pressed, simulates the key combination R-Shift + R + Delete to cancel animations.

## Usage

To use this tool, you can run it from the command line with the following options:

- `-h` : Prints help text.
- `-r` : Records your selected key for animation canceling and changes it to the pressed key.
- `-e` : Resets the configuration to the default settings.
- `--start` : Starts the executable as a service.
- `--stop` : Stops the service.
- `--service` : This flag indicates that the executable is intended to run as a service. Use `--start` instead. Running this argument from terminal is not recommended

### Example Usage


# Display the help text
./SVAnimCancel -h

# Record and set a new key for animation canceling
./SVAnimCancel -r

# Reset the configuration to default settings
./SVAnimCancel -e

# Start the tool as a service
./SVAnimCancel --start

# Stop the service
./SVAnimCancel --stop

## Running as a Service
If you want to run this tool as a service, you can use the `--start` and `--stop` options. Make sure to specify `--start` to initiate the service and `--stop` to stop it.

**Note:** If you use the `--service` flag, it indicates that the executable should run as a service, but it's recommended to use `--start` instead of running it  `--service` in terminal as it works unexpected.


