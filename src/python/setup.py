from distutils.core import setup, Extension


setup(name = "mylib",
		version = "1.0",
		description = "print log",
		author = "kinow",
		author_email = "seokju.kang@secui.com",
		url = "github.com/kinow83",
		ext_modules = [Extension("mylib", ["mylib.c"])]
	 )
