
require 'rubygems'
Gem::manage_gems
require 'rake/gempackagetask'
require 'rake/clean'
require 'rake/rdoctask'

NAME           = 'bitpack'
VERS           = '0.1'
GEM_NAME       = "#{NAME}-#{VERS}.gem"
TGZ_NAME       = "#{NAME}-#{VERS}.tgz"
RUBYFORGE_USER = "burrows"
WWW            = "#{RUBYFORGE_USER}@rubyforge.org:/var/www/gforge-projects/#{NAME}/"

RDOC_MAIN = "README"

spec = Gem::Specification.new do |s|
  s.name             = NAME
  s.version          = VERS
  s.author           = "Corey Burrows"
  s.email            = "corey.burrows@gmail.com"
  s.platform         = Gem::Platform::RUBY
  s.summary          = "Library for packing and unpacking binary strings."
  s.files            = %w{README CHANGELOG LICENSE Rakefile} +
                       Dir.glob("ext/**/*.{h,c,rb}") +
                       Dir.glob("lib/**/*.{rb}") +
                       Dir.glob("test/**/*.rb")
  s.require_path     = "."
  s.autorequire      = "bitpack"
  s.extensions       = ["ext/extconf.rb"]
  #s.test_file        = ""
  s.has_rdoc         = true
  s.extra_rdoc_files = [ RDOC_MAIN, "CHANGELOG", "LICENSE" ]
end

Rake::GemPackageTask.new(spec) do |pkg|
  pkg.need_tar = true
end

Rake::RDocTask.new(:rdoc) do |rd|
  rd.main = RDOC_MAIN
  rd.rdoc_files.include(RDOC_MAIN, "CHANGELOG", "LICENSE", "ext/**/*.c", "lib/**/*.rb")
  rd.options << "--all"
end

CLEAN.include FileList["ext/**/*.o",
                       "ext/**/*.so",
                       "ext/**/*.bundle",
                       "ext/**/Makefile",
                       "ext/**/mkmf.log",
                       "pkg/*",
                       "test/*.o",
                       "test/test_driver",
                       "html"
                      ]

task :build do
  Dir.chdir('ext')
  sh('ruby extconf.rb')
  sh('make')
  Dir.chdir('..')
end

task :c_test do
  Dir.chdir('test')
  sh('make')
  Dir.chdir('..')
end

task :ruby_test do
  Dir.chdir('test')
  sh('ruby bitpack_tests.rb')
  Dir.chdir('..')
end

task :test => [ :build, :c_test, :ruby_test ] do
end

task :gem do
  sh %{rake pkg/#{GEM_NAME}}
end

task :tgz do
  sh %{rake pkg/#{TGZ_NAME}}
end

task :package => [ :gem, :tgz ]

task :install => :gem do
  sh %{sudo gem install pkg/#{GEM_NAME}}
end

task :uninstall do
  sh %{sudo gem uninstall #{NAME}}
end

task :www => :rdoc do
  sh("scp -r html/* #{WWW}")
end

