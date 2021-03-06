/**
 * @file main.c
 *
 * @author Nilo Redini
 * @author Francesco Disperati
 * @author Davide Pellegrino
 *
 * Y.A.L.Z.I.: yet another LZ78 implementation.
 *
*/

#include <stdio.h>
#include <fcntl.h>
#include <getopt.h>

#include "YLZoptions.h"
#include "YLZhashtable.h"
#include "bitio.h"
#include "YLZcompressor.h"
#include "YLZdecompressor.h"

#define VERSION "0.1.0"

void version()
{
  fprintf(stdout, "Yalzi v%s, (C) 2012-2013 n.redini@gmail.com me@nebirhos.com pelldav@gmail.com\n", VERSION);
}

void usage()
{
  fprintf(stdout, "Usage:\n  yalzi -[c|x] FILE [OUT_FILE]\n\n");
  fprintf(stdout, "Options:\n");
  fprintf(stdout, "  -c, [--compress=FILE]            # Compress FILE to OUT_FILE [default OUT_FILE: FILE.ylz]\n");
  fprintf(stdout, "  -x, [--extract=FILE]             # Extract from FILE to OUT_FILE\n");
  fprintf(stdout, "  -v, [--verbose]                  # Print useless info\n");
  fprintf(stdout, "  -h, [--help]                     # Show this help message and quit\n");
  fprintf(stdout, "  -V, [--version]                  # Show Yalzi version number and quit\n");
}


int main(int argc, char *argv[])
{
  char *archive_path = NULL, *file_path = NULL;
  char mode = 0;
  BITIO* in_file = NULL, *out_file = NULL;
  int i;
  int cst_alloc_fp, cst_alloc_ap;

  verbose_flag = 0;
  cst_alloc_fp = 0;
  cst_alloc_ap = 0;

  while (1) {
    static struct option long_options[] = {
      /* These options don't set a flag.
         We distinguish them by their indices*/
      {"compress", required_argument, 0, 'c'},
      {"extract",  required_argument, 0, 'x'},
      {"help",     no_argument,       0, 'h'},
      {"version",  no_argument,       0, 'V'},
      /* These options set a flag. */
      {"verbose",  no_argument, &verbose_flag, 1},
      {0, 0, 0, 0}
    };
    /* getopt_long stores the option index here. */
    int option_index = 0;

    i = getopt_long(argc, argv, "c:x:vhV",
                    long_options, &option_index);

    /* Detect the end of the options. */
    if (i == -1)
      break;

    switch (i) {
    case 0:
      /* This option sets a flag. */
      break;

    case 'v':
      verbose_flag = 1;
      break;

    case 'c':
      mode = i;
      file_path = optarg;
      break;

    case 'x':
      mode = i;
      archive_path = optarg;
      break;

    case 'V':
      version();
      return 0;

    case 'h':
    case '?':
    default:
      /* getopt_long already printed an error message. */
      usage();
      return 0;
    }
  }

  /* Check options */
  if (verbose_flag)
    fprintf(stdout, "Verbose flag is set\n");

  if (mode == 'c') {
    /* Check if ARCHIVE path is passed */
    if (optind < argc)
      archive_path = argv[optind];
    else{
      cst_alloc_ap = 1;
      if (asprintf(&archive_path, "%s.ylz", file_path) == -1) {
        fprintf(stderr, "Error: unable to allocate memory!\n");
        if (archive_path != NULL)
          free(archive_path);
        return -1;
      }

    }
  }
  else if (mode == 'x') {
    /* Check if FILE path is passed */
    if (optind < argc) {
      file_path = argv[optind];
    }
    else {
      cst_alloc_fp = 1;
      if (asprintf(&file_path, "%s.out", archive_path) == -1) {
        fprintf(stderr, "Error: unable to allocate memory!\n");
        if (file_path != NULL)
          free(file_path);
        return -1;
      }
      char* extension = strstr(file_path, ".ylz");
      if (extension)
        *extension = '\0';
    }
  }
  else {
    /* no mode specified */
    fprintf(stderr, "Error: no mode has been specified!\n");
    usage();
    return -1;
  }

  print_verbose("File path: %s\nArchive path: %s\n", file_path, archive_path);

  /* Compression */
  if (mode == 'c') {
    print_verbose("Compression mode\n");
    if ((in_file = bitio_open(file_path, O_RDONLY)) == NULL) {
      fprintf(stderr, "Error: cannot open %s!\n", file_path);
      if (cst_alloc_ap)
        free(archive_path);
      return -1;
    }
    if ((out_file = bitio_open(archive_path, O_WRONLY)) == NULL) {
      fprintf(stderr, "Error: cannot open %s!\n", archive_path);
      if (cst_alloc_ap)
        free(archive_path);
      return -1;
    }

    compress(in_file, out_file);
    bitio_close(in_file);
    bitio_close(out_file);

    if (cst_alloc_ap)
      free(archive_path);
  }

  /* Decompression */
  else if (mode == 'x') {
    print_verbose("Deompression mode\n");
    if ((in_file = bitio_open(archive_path, O_RDONLY)) == NULL) {
      fprintf(stderr, "Error: cannot open %s!\n", archive_path);
      if (cst_alloc_fp)
        free(file_path);
      return -1;
    }
    if ((out_file = bitio_open(file_path, O_WRONLY)) == NULL) {
      fprintf(stderr, "Error: cannot open %s!\n", file_path);
      if (cst_alloc_fp)
        free(file_path);
      return -1;
    }

    decompress(in_file, out_file);
    bitio_close(in_file);
    bitio_close(out_file);

    if (cst_alloc_fp)
      free(file_path);
  }

  return 0;
}
