# Out-of-tree drivers vendored into this config repo (registered as a Zephyr module
# via zephyr/module.yml -> build.cmake). Only compiled when their CONFIG_* is set,
# so boards that don't opt in are unaffected.
add_subdirectory(drivers)
