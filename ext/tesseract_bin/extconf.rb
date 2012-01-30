require 'mkmf'

root = File.expand_path('../../..', __FILE__)

Dir.chdir(File.join(root, 'vendor/tesseract-2.04')) do
  system "./configure", "--prefix=#{root}", "--exec-prefix=#{root}"
  system "make"
  #system "make install"
#  puts "make clean"
#  system "make clean"
end

create_makefile 'tesseract_bin'
