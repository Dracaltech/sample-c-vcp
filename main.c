#include <stdio.h>    // standard input / output functions
#include <stdlib.h>   // general purpose functions
#include <string.h>   // string function definitions
#include <unistd.h>   // UNIX standard function definitions
#include <fcntl.h>    // File control definitions
#include <errno.h>    // Error number definitions
#include <termios.h>  // POSIX terminal control definitions
#include <stdbool.h>  // Boolean types and values
#include <time.h>     // Timekeeping types and functions
#include <regex.h>    // GNU Regular expression definitions

#include "checksum.h"   // CRC calculation library from github.com/lammertb/libcrc

/**
 * Path to the file descriptor of the port to be read from
 * MacOS: /dev/tty.usbmodem[serial of Dracal device]1
 * Linux: /dev/ttyACM[number]
**/
const char* dev = "/dev/ttyACM0";

int read_line(int fd, char* line) {

  // Allocate memory for read buffer
  char buf[256]; // Read buffer
  memset(buf, '\0', sizeof buf);

  // Loop until a complete line is read
  // Note: A full CRLF is expected but in case the CR is ignored we wait for the LF only
  while (!strstr(line, "\n")) {

    // Read the port's content to buf
    if (read(fd, buf, sizeof buf) < 0) {
      if (errno == EAGAIN) {
        // This only means the port had no data
        usleep(100000); // retry in 100 ms
        continue;
      }
      else {
        return -1;
      }
    }

    if (*buf != '\0') {
      strcat(line, buf);
      memset(buf, '\0', sizeof buf);
    }
  }

  // Variables necessary to the integrity check
  uint16_t crc;       // Read checksum value
  char* sep;          // Position of the asterisk in the line
  static regex_t re;  // Pattern to match to the expected content of a line
  static bool is_compiled = false;

  if (!is_compiled) {
    regcomp(&re, "^[^\\*]+\\*[0-9a-f]{4}\\s*$", 0);
    is_compiled = true;
  }

  // Filter out lines whose format would crash the CRC check, they are surely invalid
  if (regexec(&re, line, 0, NULL, 0)) {

    sep = strchr(line, '*');
    crc = strtol(sep + 1, NULL, 16);

    // CRC validation
    if (crc == crc_xmodem((unsigned char*)line, (size_t)(sep - line))) {
      *sep = '\0'; // Replace the * with a null character, now line stops at the end of the content
      return 0;
    }
  }

  // We will get here if the checks failed
  printf("Integrity error: %s\n", line);
  return 0;
}

/**
 * This function opens a connection, sets the necessary settings and
 * returns a file descriptor with which data can be read from or sent to.
 * dev is a string of the path to the device to converse with such as
 *
**/
int open_port(const char* dev) {

  // Open the file and get the descriptor
  int fd = open(dev, O_RDWR | O_NOCTTY | O_NDELAY);
  if (fd < 0) {
    perror("Error opening file");
    return -1;
  }

  // Configure Port
  struct termios options;
  memset(&options, 0, sizeof options);

  // Get the current options
  if (tcgetattr(fd, &options) != 0) {
    perror("Error in tcgettattr");
    return -1;
  }

  // Set Baud Rate
  cfsetospeed(&options, B9600);
  cfsetispeed(&options, B9600);

  // Setting other options
  options.c_cflag &= ~(PARENB | CSTOPB);              // No parity, 1 stop bit
  options.c_cflag &= ~CSIZE;                          // Charater size mask
  options.c_cflag |= CS8;                             // 8 bits
  options.c_cflag &= ~CRTSCTS;                        // No flow control
  options.c_cflag |= CREAD | CLOCAL;                  // Turn on READ & ignore ctrl lines

  options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG); // Raw input

  options.c_iflag &= ~(IXON | IXOFF | IXANY);         // Turn off software flow contrl
  options.c_iflag &= ~IGNCR;                          // Don't ignore CR character

  options.c_oflag &= ~OPOST;                          // Don't replace outgoing LF with CRLF - for clarity they are explicit here

  // Flush port
  if (tcflush(fd, TCIFLUSH) != 0) {
    perror("Error in tcflush");
    return -1;
  }
  // Apply attributes
  if (tcsetattr(fd, TCSANOW, &options) != 0) {
    perror("Error in tcsettattr");
    return -1;
  }
  return fd;
}

int main(int argc, char** argv) {
  // ignore unused
  (void)argc; (void)argv;

  // Open File Descriptor
  int fd = open_port(dev);
  if (fd < 0) {
    return EXIT_FAILURE;
  }

  // Defining commands as strings is convenient for the use of the sizeof operator
  const unsigned char poll_cmd[] = "POLL 1000\r\n";

  // Variables for timekeeping
  time_t t;
  struct tm* localt;
  char timestr[20];

  // Variables related to line manipulation
  char line[256];    // Contents of the active line

  // Variables containing processed data from the device.
  // This was made with a PTH sensor in mind, other sensor types will need different declarations
  int    pressure;      // Pressure in Pascals
  float  temperature;   // Temperature in Celsius
  float  humidity;      // Humidity in %
  char   model[32];     // Model id of device
  char   serial[7];     // Serial number of device
  char   message[128];  // Message contained for info lines

  bool info_line_read = false;

  // Could be any number or a while loop, change as needed.
  // i variable is not used but could be useful for unique line IDs
  for (int i = 0; i < 10; i++) {

    // Set the poll rate if it has not been set yet.
    if (!info_line_read) {
      if (write(fd, poll_cmd, sizeof(poll_cmd) - 1) < 0) {
        perror("Error writing");
      }
    }

    // (Re)initialize line
    memset(line, '\0', sizeof line);

    // Wait until a full line has been read and validated
    if (read_line(fd, line) < 0) {
      perror("Error reading");
    }

    // Here we generate a string to represent the time at which the line was recieved
    t = time(NULL);
    localt = localtime(&t);
    strftime(timestr, 20, "%F %T", localt); // YYYY-MM-DD HH:MM:SS


    if (line[0] == 'I') {
      // For info lines (the POLL response in this case)
      sscanf(
        line,
        "I,%[^,],%[^,],%[^,]",
        model,
        serial,
        message
      );

      printf("\n%s\n", message);
      info_line_read = true;
    }
    else {
      /**
       * Interpret the line and save the result into the variables.
       * The format string to use would depend on the sensor, this example was made with the PTH sensor in mind.
       * Refer to these resources to learn more on how to do so
       *   Format strings for the scanf functions  : cplusplus.com/reference/cstdio/scanf/
       *   Dracal sensor VCP mode output format    : dracal.com/en/usage-guides/vcp_howto
      **/
      sscanf(line, "%*c,%*[^,],%*[^,],,%i,Pa,%f,C,%f,%%", &pressure, &temperature, &humidity);

      // This is where you would put your own code to be executed on data.
      printf(
        "\n%s %s @ %s\nP = %i Pa\nT = %.2f C\nH = %.2f %%\n",
        model, serial, timestr,
        pressure, temperature, humidity
      );
    }
  }

  close(fd); // Close the serial port

  return EXIT_SUCCESS;
}
