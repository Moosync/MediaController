{
	'targets': [
		{
			'target_name': 'MediaController',
			'sources': [],
			"conditions": [
				["OS=='win'", {
					'include_dirs': ["<!@(node -p \"require('node-addon-api').include\")"],
					'cflags!': ['-fno-exceptions'],
					'cflags_cc!': ['-fno-exceptions'],
					'sources': [ 'src/index.cc', 'src/utils.cc', 'src/player.cc' ],
					'libraries': [ 'WindowsApp.lib' ],
					'msvs_settings': {
						'VCCLCompilerTool': {
							'ExceptionHandling': 1,
							'AdditionalOptions': ['/std:c++20', '/await', '/Zc:twoPhase-'],
						},
					},
				}]
			]
		}
	]
}