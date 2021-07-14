import argparse
import os
import zipfile

def make_rel_archive(a_args):
	archive = zipfile.ZipFile("(Part 1) Engine Fixes.zip".format(a_args.name), "w", zipfile.ZIP_DEFLATED)
	def do_write(a_path):
		archive.write(a_path, "SKSE/Plugins/{}".format(os.path.basename(a_path)))
	def write_rootfile(a_extension):
		do_write("{}/{}{}".format(a_args.src_dir, a_args.name, a_extension))

	do_write(a_args.dll)
	write_rootfile("_preload.txt")
	write_rootfile("_SNCT.toml")
	write_rootfile(".toml")

def make_dbg_archive(a_args):
	archive = zipfile.ZipFile("{}_pdb.zip".format(a_args.name), "w", zipfile.ZIP_DEFLATED)
	archive.write(a_args.pdb, os.path.basename(a_args.pdb))

def parse_arguments():
	parser = argparse.ArgumentParser(description="archive build artifacts for distribution")
	parser.add_argument("--dll", type=str, help="the full dll path", required=True)
	parser.add_argument("--name", type=str, help="the project name", required=True)
	parser.add_argument("--out-dir", type=str, help="the output directory", required=True)
	parser.add_argument("--pdb", type=str, help="the full pdb path", required=True)
	parser.add_argument("--src-dir", type=str, help="the project root source directory", required=True)
	return parser.parse_args()

def main():
	args = parse_arguments()

	os.makedirs(args.out_dir, exist_ok=True)
	os.chdir(args.out_dir)

	make_rel_archive(args)
	make_dbg_archive(args)

if __name__ == "__main__":
	main()
