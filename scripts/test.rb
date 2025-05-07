#!/usr/bin/env ruby
require 'cgi'

cgi = CGI.new
query_string = ENV['QUERY_STRING'] || ""

puts cgi.header("type" => "text/html", "charset" => "UTF-8")

puts "<!-- QUERY_STRING: #{query_string} -->"
puts "<!DOCTYPE html>"
puts "<html lang='fr'>"
puts "<head>"
puts "  <meta charset='UTF-8'>"
puts "  <title>Calculatrice CGI</title>"
puts "</head>"
puts "<body>"

a = cgi['a']
b = cgi['b']
op = cgi['op']

if a != "" && b != "" && op != ""
  begin
    x = Float(a)
    y = Float(b)
    result = case op
             when "+" then x + y
             when "-" then x - y
             when "*" then x * y
             when "/" then y != 0 ? x / y : "Erreur : division par zéro"
             else "Opérateur inconnu"
             end
    puts "<h1>Résultat : #{x} #{op} #{y} = #{result}</h1>"
  rescue
    puts "<h1>Erreur : Veuillez entrer deux nombres valides.</h1>"
  end
else
  puts "<h1>Bienvenue sur la mini-calculatrice CGI</h1>"
end

puts "<form method='get' action=''>"
puts "  <input type='text' name='a' placeholder='Nombre 1' />"
puts "  <select name='op'>"
puts "    <option value='+'>+</option>"
puts "    <option value='-'>-</option>"
puts "    <option value='*'>*</option>"
puts "    <option value='/'>/</option>"
puts "  </select>"
puts "  <input type='text' name='b' placeholder='Nombre 2' />"
puts "  <input type='submit' value='Calculer' />"
puts "</form>"

puts "</body>"
puts "</html>"
