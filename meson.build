project(
  'tcp-speed-asio',
  'cpp',
  default_options: ['cpp_std=c++17', 'warning_level=3'],
  license: 'MIT',
  meson_version: '>=0.53.2')

boost_dep = dependency('Boost', modules: ['coroutine'], version: '>=1.71.0')
threads_dep = dependency('threads')
deps = [boost_dep, threads_dep]

executable('server', 'server.cpp', dependencies: deps)
executable('client', 'client.cpp', dependencies: deps)
