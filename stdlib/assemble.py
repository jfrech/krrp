import os


module_name_aliases = {
    'list': ('L',),
    'tuple': ('T',),
    'prelude': ('P',),
    'function': ('F',),
    'math': ('M',),
}


stdlib = os.path.dirname(os.path.realpath(__file__))

template = 'atom_scope_push(ImportedSource, atom_name_new(strdup(%s)), atom_string_newfl(%s));'
c_escape = lambda s: r'(char []) {%s, 0x00}' % ', '.join(fr'0x{ord(c) & 0xFF:X}' for c in s)

with open(stdlib + '/krrp_stdlib.c', 'w') as krrp_stdlib:
    for _file_name in os.listdir(stdlib):
        full_file_name = stdlib + '/' + _file_name
        file_name, extension = os.path.splitext(_file_name)
        module_name = os.path.basename(file_name)

        if extension == '.krrp':
            with open(full_file_name, 'r') as f:
                module_content = f.read()
                names = (module_name, ) + module_name_aliases.get(module_name, ())

                for name in names:
                    krrp_stdlib.write(template % (c_escape(name), c_escape(module_content)) + '\n')
