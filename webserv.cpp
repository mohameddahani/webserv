/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   webserv.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mdahani <mdahani@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/26 08:46:45 by mdahani           #+#    #+#             */
/*   Updated: 2025/12/26 09:17:09 by mdahani          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "includes/webserv.hpp"

void Webserv::initMimeTypes() {
  // * text
  this->mimeTypes[".html"] = "text/html";
  this->mimeTypes[".htm"] = "text/html";
  this->mimeTypes[".shtml"] = "text/html";
  this->mimeTypes[".css"] = "text/css";
  this->mimeTypes[".xml"] = "text/xml";
  this->mimeTypes[".mml"] = "text/mathml";
  this->mimeTypes[".txt"] = "text/plain";
  this->mimeTypes[".jad"] = "text/vnd.sun.j2me.app-descriptor";
  this->mimeTypes[".wml"] = "text/vnd.wap.wml";
  this->mimeTypes[".htc"] = "text/x-component";

  // * images
  this->mimeTypes[".gif"] = "image/gif";
  this->mimeTypes[".jpeg"] = "image/jpeg";
  this->mimeTypes[".jpg"] = "image/jpeg";
  this->mimeTypes[".avif"] = "image/avif";
  this->mimeTypes[".png"] = "image/png";
  this->mimeTypes[".svg"] = "image/svg+xml";
  this->mimeTypes[".svgz"] = "image/svg+xml";
  this->mimeTypes[".tif"] = "image/tiff";
  this->mimeTypes[".tiff"] = "image/tiff";
  this->mimeTypes[".wbmp"] = "image/vnd.wap.wbmp";
  this->mimeTypes[".webp"] = "image/webp";
  this->mimeTypes[".ico"] = "image/x-icon";
  this->mimeTypes[".jng"] = "image/x-jng";
  this->mimeTypes[".bmp"] = "image/x-ms-bmp";

  // * fonts
  this->mimeTypes[".woff"] = "font/woff";
  this->mimeTypes[".woff2"] = "font/woff2";
  this->mimeTypes[".eot"] = "application/vnd.ms-fontobject";

  // * application
  this->mimeTypes[".js"] = "application/javascript";
  this->mimeTypes[".atom"] = "application/atom+xml";
  this->mimeTypes[".rss"] = "application/rss+xml";
  this->mimeTypes[".jar"] = "application/java-archive";
  this->mimeTypes[".war"] = "application/java-archive";
  this->mimeTypes[".ear"] = "application/java-archive";
  this->mimeTypes[".json"] = "application/json";
  this->mimeTypes[".hqx"] = "application/mac-binhex40";
  this->mimeTypes[".doc"] = "application/msword";
  this->mimeTypes[".pdf"] = "application/pdf";
  this->mimeTypes[".ps"] = "application/postscript";
  this->mimeTypes[".eps"] = "application/postscript";
  this->mimeTypes[".ai"] = "application/postscript";
  this->mimeTypes[".rtf"] = "application/rtf";
  this->mimeTypes[".m3u8"] = "application/vnd.apple.mpegurl";
  this->mimeTypes[".kml"] = "application/vnd.google-earth.kml+xml";
  this->mimeTypes[".kmz"] = "application/vnd.google-earth.kmz";
  this->mimeTypes[".xls"] = "application/vnd.ms-excel";
  this->mimeTypes[".ppt"] = "application/vnd.ms-powerpoint";
  this->mimeTypes[".odg"] = "application/vnd.oasis.opendocument.graphics";
  this->mimeTypes[".odp"] = "application/vnd.oasis.opendocument.presentation";
  this->mimeTypes[".ods"] = "application/vnd.oasis.opendocument.spreadsheet";
  this->mimeTypes[".odt"] = "application/vnd.oasis.opendocument.text";
  this->mimeTypes[".pptx"] =
      "application/"
      "vnd.openxmlformats-officedocument.presentationml.presentation";
  this->mimeTypes[".xlsx"] =
      "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet";
  this->mimeTypes[".docx"] =
      "application/vnd.openxmlformats-officedocument.wordprocessingml.document";
  this->mimeTypes[".wmlc"] = "application/vnd.wap.wmlc";
  this->mimeTypes[".wasm"] = "application/wasm";
  this->mimeTypes[".7z"] = "application/x-7z-compressed";
  this->mimeTypes[".cco"] = "application/x-cocoa";
  this->mimeTypes[".jardiff"] = "application/x-java-archive-diff";
  this->mimeTypes[".jnlp"] = "application/x-java-jnlp-file";
  this->mimeTypes[".run"] = "application/x-makeself";
  this->mimeTypes[".pl"] = "application/x-perl";
  this->mimeTypes[".pm"] = "application/x-perl";
  this->mimeTypes[".prc"] = "application/x-pilot";
  this->mimeTypes[".pdb"] = "application/x-pilot";
  this->mimeTypes[".rar"] = "application/x-rar-compressed";
  this->mimeTypes[".rpm"] = "application/x-redhat-package-manager";
  this->mimeTypes[".sea"] = "application/x-sea";
  this->mimeTypes[".swf"] = "application/x-shockwave-flash";
  this->mimeTypes[".sit"] = "application/x-stuffit";
  this->mimeTypes[".tcl"] = "application/x-tcl";
  this->mimeTypes[".tk"] = "application/x-tcl";
  this->mimeTypes[".der"] = "application/x-x509-ca-cert";
  this->mimeTypes[".pem"] = "application/x-x509-ca-cert";
  this->mimeTypes[".crt"] = "application/x-x509-ca-cert";
  this->mimeTypes[".xpi"] = "application/x-xpinstall";
  this->mimeTypes[".xhtml"] = "application/xhtml+xml";
  this->mimeTypes[".xspf"] = "application/xspf+xml";
  this->mimeTypes[".zip"] = "application/zip";

  // * octet-stream (fallback binaries)
  this->mimeTypes[".bin"] = "application/octet-stream";
  this->mimeTypes[".exe"] = "application/octet-stream";
  this->mimeTypes[".dll"] = "application/octet-stream";
  this->mimeTypes[".deb"] = "application/octet-stream";
  this->mimeTypes[".dmg"] = "application/octet-stream";
  this->mimeTypes[".iso"] = "application/octet-stream";
  this->mimeTypes[".img"] = "application/octet-stream";
  this->mimeTypes[".msi"] = "application/octet-stream";
  this->mimeTypes[".msp"] = "application/octet-stream";
  this->mimeTypes[".msm"] = "application/octet-stream";

  // * audio
  this->mimeTypes[".mid"] = "audio/midi";
  this->mimeTypes[".midi"] = "audio/midi";
  this->mimeTypes[".kar"] = "audio/midi";
  this->mimeTypes[".mp3"] = "audio/mpeg";
  this->mimeTypes[".ogg"] = "audio/ogg";
  this->mimeTypes[".m4a"] = "audio/x-m4a";
  this->mimeTypes[".ra"] = "audio/x-realaudio";

  // * video
  this->mimeTypes[".3gp"] = "video/3gpp";
  this->mimeTypes[".3gpp"] = "video/3gpp";
  this->mimeTypes[".ts"] = "video/mp2t";
  this->mimeTypes[".mp4"] = "video/mp4";
  this->mimeTypes[".mpeg"] = "video/mpeg";
  this->mimeTypes[".mpg"] = "video/mpeg";
  this->mimeTypes[".mov"] = "video/quicktime";
  this->mimeTypes[".webm"] = "video/webm";
  this->mimeTypes[".flv"] = "video/x-flv";
  this->mimeTypes[".m4v"] = "video/x-m4v";
  this->mimeTypes[".mng"] = "video/x-mng";
  this->mimeTypes[".asx"] = "video/x-ms-asf";
  this->mimeTypes[".asf"] = "video/x-ms-asf";
  this->mimeTypes[".wmv"] = "video/x-ms-wmv";
  this->mimeTypes[".avi"] = "video/x-msvideo";
}
