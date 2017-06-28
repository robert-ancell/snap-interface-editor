project ('snap-interface-editor', ['c'],
         version: '0',
         license: 'GPLv3+',
         default_options: [
             'warning_level=1',
             'buildtype=debugoptimized',
         ],
         meson_version: '>= 0.37.1')

gnome = import ('gnome')

# Dependencies
glib_dep = dependency ('glib-2.0', version: '>= 2.38')
gtk_dep = dependency ('gtk+-3.0', version: '>=3.12')
snapd_glib_dep = dependency ('snapd-glib')

snap_interface_editor = executable ('snap-interface-editor',
                                    [ 'snap-interface-editor.c' ],
                                    dependencies: [ glib_dep, gtk_dep, snapd_glib_dep ],
                                    install: true)
