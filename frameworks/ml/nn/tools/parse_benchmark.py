#!/usr/bin/python3
""" NNAPI benchmark output parser (statistics aggregator)

Reads a file with output from multiple runs of
  adb shell am instrument
    -w com.android.nn.benchmark.app/androidx.test.runner.AndroidJUnitRunner

and provides either raw measurements or aggregated statistics of the runs.

Usage:
  parse_benchmark --format=[json|table] --output=[full|stats] [adb output filename]

"""
import argparse
import json
import statistics

FLAG_FORMAT_JSON = "json"
FLAG_FORMAT_TABLE = "table"

FLAG_OUTPUT_FULL = "full"
FLAG_OUTPUT_STATS = "stats"


def main():
  parser = argparse.ArgumentParser()
  parser.add_argument("input", help="input filename")
  parser.add_argument("--format", help="output format ({0}|{1})"
                      .format(FLAG_FORMAT_TABLE, FLAG_FORMAT_JSON),
                      default=FLAG_FORMAT_TABLE)
  parser.add_argument("--output", help="output data ({0}|{1})"
                      .format(FLAG_OUTPUT_FULL, FLAG_OUTPUT_STATS),
                      default=FLAG_OUTPUT_STATS)
  args = parser.parse_args()

  data = read_data(args.input)

  if args.output == FLAG_OUTPUT_STATS:
    stats = compute_stats(data)
    print_stats(stats, args.format)
  else:
    print_data(data, args.format)


def read_data(input_filename):
  data = dict()

  with open(input_filename) as f:
    for line in f:
      if "INSTRUMENTATION_STATUS:" in line and "_avg" in line:
        sample = line.split(": ")[1]
        name, value = sample.split("=")
        name = name[:-4]
        data[name] = data.get(name, []) + [float(value)]

  return data


def compute_stats(data):
  stats = list()

  for name in sorted(data):
    values = data[name]
    stat_mean = statistics.mean(values)
    stat_stdev = statistics.stdev(values)
    stat_min = min(values)
    stat_max = max(values)
    stat_n = len(values)
    stats.append({"benchmark": name, "mean": stat_mean, "stddev": stat_stdev,
                  "min": stat_min, "max": stat_max, "n": stat_n})

  return stats


def print_stats(stats, print_format):
  if print_format == FLAG_FORMAT_TABLE:
    print("{0:<34}{1:>10}{2:>10}{3:>10}{4:>10}{5:>10}".format(
        "Benchmark", "mean", "stddev", "min", "max", "n"))
    for line in stats:
      print("{0:<34}{1:>10.2f}{2:>10.2f}{3:>10.2f}{4:>10.2f}{5:>10d}".format(
          line["benchmark"], line["mean"], line["stddev"], line["min"],
          line["max"], line["n"]))
  else:
    print(json.dumps(stats))


def print_data(data, print_format):
  if print_format == FLAG_FORMAT_TABLE:
    print("{0:<34}{1:>10}".format(
        "Benchmark", "sample"))

  for name in data:
    for sample in data[name]:
      if print_format == FLAG_FORMAT_TABLE:
        print("{0:<34}{1:>10}".format(name, sample))
      else:
        print(json.dumps({"benchmark": name, "sample": sample}))

if __name__ == "__main__":
  main()
