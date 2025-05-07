project('project_rsa', 'c')

deps = [
  dependency('gmp'),
  subproject('facil').get_variable('facil_dep'),
]

src = [
  'main.c',
  'rsa.c',
]

executable('rsa', 
  sources : src, 
  dependencies : deps,
)
