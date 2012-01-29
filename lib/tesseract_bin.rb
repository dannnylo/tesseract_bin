require 'pathname'

module TesseractBin
  Root = Pathname.new(File.expand_path('../..', __FILE__))
#  Bin  = Root.join('bin')

#  Executables = Bin.children.inject({}) { |h, p|
#    h[p.basename.to_s.to_sym] = p.to_s
#    h
#  }
end

