#!/usr/bin/env ruby

# En-têtes HTTP : Content-Type et une ligne vide pour signaler la fin des headers
puts "Content-Length: #{File.size(__FILE__)}"
puts "Connection: close\r\n"
puts "Content-Type: text/html\r\n\r\n"

# Le corps HTML
puts "<h1>Hello from Ruby CGI!</h1>\r\n"
