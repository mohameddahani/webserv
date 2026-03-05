#!/usr/bin/env ruby

# CGI headers
puts "Content-Type: text/html"
puts ""  # Blank line to separate headers from body

# HTML output
puts "<html>"
puts "<head><title>Ruby CGI Test</title></head>"
puts "<body>"
puts "<h1>Hello from Ruby CGI! </h1>"
puts "<p>If you see this, Ruby CGI is working!</p>"
puts "<p>Current time: #{Time.now}</p>"
puts "</body>"
puts "</html>"