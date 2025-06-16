project ('char_db', 'cpp',
         version: '0.1.0',
         license: 'AGPL',
         default_options: ['cpp_std=c++23'],)

python3 = find_program ('python3', required: true)

ucd_incs = ['just_ranges.inc',
            'bmp_ranges.inc',
            'non_bmp_ranges.inc',
            'utf8_ranges_1.inc',
            'utf8_ranges_2.inc',
            'utf8_ranges_3.inc',
            'utf8_ranges_4.inc',]
ucd_incs_extradir = meson.project_build_root () / 'generated'

ucd_gen = custom_target (
    'ucd_gen',
    input: 'tools/ucd_gen.py',
    output: ucd_incs,
    command: [python3, '@INPUT@', '@OUTDIR@/ucd', '@OUTDIR@', ucd_incs_extradir],
    install: true,
    install_dir: get_option ('includedir') / 'char_db/generated',)

install_subdir ('include/char_db', install_dir: get_option ('includedir'))

char_db_dep = declare_dependency (
    include_directories: [include_directories ('include'),
                          meson.project_build_root (),],
    sources: ucd_gen,)