require 'mkmf'

root = File.expand_path('../../..', __FILE__)

Dir.chdir(File.join(root, 'vendor/tesseract-2.04')) do
  system "./configure", "--prefix=#{root} --exec-prefix=#{root}"
  puts "make"
  system "make"
  puts "make install"
  system "make install"
  puts root.inspect
#  puts "make clean"
#  system "make clean"
end

create_makefile 'tesseract_bin'

