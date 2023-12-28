from PIL import Image
from pathlib import Path
import argparse
import sys
import re

# Notes:
# + Font chars rows bits are padded with zeros if narrower than 8 bits.
# + Least significant bits of each byte are left on the screen, but bytes order go from left to right.
# + Font data include multiple characters data, each character data consist of font height number of rows,
#   each row consists of font width number of bits.

def eprint(*args, **kwargs):
	print(*args, file=sys.stderr, **kwargs)

if __name__ == "__main__":
	parser = argparse.ArgumentParser(description='Convert BMP or PNG files representing fonts to C/C++/Arduino arrays.')
	parser.add_argument('-i', '--input', type=str, required=True, help='Path to the BMP or PNG file')
	parser.add_argument('-o', '--output', type=str, help='Path to output file, dash (-) to output to standard output or empty/none to use basename of input with header extension')
	parser.add_argument('-s', '--start', type=str, default=' ', help='Starting character (default: space)')
	parser.add_argument('-e', '--end', type=str, default='~', help='Ending character (default: tilde)')
	parser.add_argument('-x', '--width', type=int, default=0, help='Width of the character, or none to assume 16 characters in a row (default)')
	parser.add_argument('-y', '--height', type=int, required=True, help='Height of the character')
	parser.add_argument('-b', '--border', type=int, default=0, help='Border in pixels around each characters (default: 0)')
	parser.add_argument('--format', type=str, default='short', choices=['minified', 'short', 'long'], help='Format of the generated code, longer meaning more human friendly')
	parser.add_argument('--language', type=str, default='arduino', choices=['arduino', 'c', 'cpp'], help='What language and/or environment the generated code is intended')
	parser.add_argument('--var', type=str, help='Name of the array variable (default basename of output file).')
	parser.add_argument('--channel', type=str, default='g', choices=['r', 'g', 'b'], help='Which color channel to use for monochrome')
	parser.add_argument('--padding', type=str, default='row', choices=['none', 'row', 'char'], help='Level where the paddings are to be applied to round up to full bytes')
	parser.add_argument('--bit-order', type=str, default='lsb', choices=['lsb', 'msb'], help='Order of bits to pixels, LSB (default) or MSB being first pixel from left')
	parser.add_argument('--xd', action='store_true', help='Adds my project-specific width and height as header before the font data')

	args = parser.parse_args()

	img = Image.open(args.input)
	r, g, b = img.split()
	if args.channel == 'r':
		img = r
	elif args.channel == 'g':
		img = g
	elif args.channel == 'b':
		img = b
	else:
		eprint('invalid channel selected')
		exit(1)
	#img = img.convert('1') # monochrome, causes missing pixels sometimes for some reason, wtf...
	char_width = args.width if args.width > 0 else img.width // 16
	char_width -= args.border
	char_height = args.height
	#img.save('./xd.bmp') # for testing

	fallback_filename = 'font_' + Path(args.input).stem + ('.h' if args.language == 'c' else '.hpp')
	if args.output == '-':
		file = sys.stdout
	else:
		if args.output:
			if Path(args.output).is_dir():
				args.output = (Path(args.output) / fallback_filename).resolve(strict=False)
				eprint(f'Directory provided for --output, saving to {args.output}')
		else:
			args.output = (Path(args.input) / '..' / fallback_filename).resolve(strict=False)
			eprint(f'No --output specified, saved to {args.output}')
		file = open(args.output, 'w')

	if not args.var:
		args.var = re.sub('[^a-zA-Z0-9_]', '_', Path(args.output).stem)

	if args.language == 'arduino':
		file.write(f'#pragma once\n\n#include <Arduino.h>\n\nconst uint8_t {args.var}[] PROGMEM = {{')
	if args.language == 'cpp':
		file.write(f'#pragma once\n\n#include <cstdint>\n\nconst uint8_t {args.var}[] = {{')
	elif args.language == 'c':
		file.write(f'#ifndef {args.var.upper()}_H\n#define {args.var.upper()}_H\n\n#include <stdint.h>\n\nconst char {args.var}[] = {{')
	
	if args.xd:
		file.write('\n\t/* width: */ '+ str(char_width) + ', /* height: */ ' + str(char_height) + ', /* bit-order: ' + args.bit_order.upper() +' */')
	elif args.format == 'long':
		file.write('\n\t/* width: '+ str(char_width) + ', height: ' + str(char_height) + ', bit-order: ' + args.bit_order.upper() + ' */')

	start_ord = ord(args.start)
	end_ord = ord(args.end)
	current_ord = 0
	for cy in range(0, img.height, char_height + args.border):
		for cx in range(0, img.width, char_width + args.border):
			if current_ord < start_ord:
				current_ord += 1
				continue
			char_bytes = []
			byte = 0
			bits_left_in_byte = 8
			if args.format == 'long':
				file.write(
					'\n\t/* ' + 
					f'index={(current_ord - start_ord)} code={current_ord} hex=0x{current_ord:02X} ' +
					(f'ascii="{chr(current_ord)}"' if current_ord >= 32 and current_ord <= 126 else '(non-ascii)') +
					' */')
			for y in range(cy, cy + char_height):
				line_bits = []
				for x in range(cx, cx + char_width):
					pixel = 1 if img.getpixel((x, y)) > 127 else 0
					line_bits.append(str(pixel))
					byte <<= 1
					byte |= pixel
					bits_left_in_byte -= 1
					if not bits_left_in_byte:
						char_bytes.append(byte)
						byte = 0
						bits_left_in_byte = 8
				if args.padding == 'row' and bits_left_in_byte != 8:
					byte <<= bits_left_in_byte
					bits_left_in_byte = 8
					char_bytes.append(byte)
					byte = 0
				if args.format == 'long':
					if args.bit_order == 'lsb':
						char_bytes = [int((f'{c:08b}')[::-1], 2) for c in char_bytes]
					file.write(
						'\n\t' + 
						(','.join([f'0x{c:02X}' for c in char_bytes])) +
						', ' + ('' if len(char_bytes) == (char_width + 7) // 8 else '     ') + '/* ' +
						' '.join(line_bits) + 
						# f' {bits_left_in_byte}' +
						' */')
					# file.write(
					# 	'\n\t' + 
					# 	(','.join([f'0b{c:08b}' for c in char_bytes])) +
					# 	', ' + ('' if len(char_bytes) == (char_width + 7) // 8 else '           ') + '/* ' +
					# 	' '.join(line_bits) + 
					# 	f' {bits_left_in_byte}' +
					# 	' */')
					char_bytes = []
			if args.padding == 'char' and bits_left_in_byte != 8:
				byte <<= bits_left_in_byte
				bits_left_in_byte = 8
				char_bytes.append(byte)
				byte = 0
			if args.format == 'short':
				if args.bit_order == 'lsb':
					char_bytes = [int((f'{c:08b}')[::-1], 2) for c in char_bytes]
				file.write(
					'\n\t' + 
					(','.join([f'0x{c:02X}' for c in char_bytes])) +
					f', /* [{(current_ord - start_ord)}] ' +
					(f"'{chr(current_ord)}'" if current_ord >= 32 and current_ord <= 126 else '???') +
					f' ({current_ord}) */')
			if current_ord is end_ord:
				if args.padding == 'none' and bits_left_in_byte != 8:
					byte <<= bits_left_in_byte
					bits_left_in_byte = 8
					char_bytes.append(byte)	
					byte = 0
				break
			current_ord += 1
		if current_ord is end_ord:
			break

	file.write("\n};\n")
	if args.language == 'c':
		file.write("#endif\n")
	file.flush()

	if file is not sys.stdout:
		file.close()
