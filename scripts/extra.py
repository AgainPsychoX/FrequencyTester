import re
import io
from contextlib import redirect_stdout
import adump

Import("env") # PlatformIO :)

def after_build(source, target, env):
	# Capture adump/objdump output 
	f = io.StringIO()
	with redirect_stdout(f):
		# 'adump' python script is used to translate addresses to names 
		# of memory mapped SFR (incl. I/O control ports) 
		adump.main([
			None, # argv[0], null, as adump is called as python module
			env['BOARD_MCU'], 
			# following are avr-objdump arguments:
			'-SC', '--file-start-context', '-w',
			env.subst('./.pio/build/${PIOENV}/${PROGNAME}.elf'),
		])
	out = f.getvalue()

	# Remove annoying hex to decimal comments
	# out = re.sub(r'\s*;\s*\d+\s*$', '', out, flags=re.MULTILINE)

	with open(env.subst('${PROGNAME}.disasm'), 'w') as file:
		file.write(out)

env.AddCustomTarget(
	'disasm',
	'${BUILD_DIR}/${PROGNAME}.elf',
	after_build,
	title='Disasm',
	description='Generate a disassembly file on demand',
	always_build=True
)

# The environment can be inspected:
# print(env.Dump())
# print(projenv.Dump())