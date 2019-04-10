import os


module_name_aliases = {
    'list': ('L',)
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

                module_loader = lambda name: template % (c_escape(name), c_escape(module_content)) + '\n'

                krrp_stdlib.write(module_loader(module_name))
                if module_name in module_name_aliases:
                    for alias in module_name_aliases[module_name]:
                        krrp_stdlib.write(module_loader(alias))
