#
#  BrainCloudSDK.podspec
#  BrainCloudSDK C++
#
#  Created by Benoit Sarrazin on Feb 16, 2016.
#  Copyright (c) 2016 BitHeads Inc. All rights reserved.
#

Pod::Spec.new do |s|

  # ―――  Spec Metadata  ―――――――――――――――――――――――――――――――――――――――――――――――――――――――――― #

  s.name     = "BrainCloudCpp"
  s.version  = "2.23.0"
  s.summary  = "The C++ client library for brainCloud"
  s.homepage = "http://getbraincloud.com/"

  s.ios.deployment_target  = "6.0"
  s.osx.deployment_target  = "10.8"
  s.tvos.deployment_target = "9.0"

  # ―――  Spec License  ――――――――――――――――――――――――――――――――――――――――――――――――――――――――――― #

  s.license                = {:type => "Apache License, Version 2.0", :file => "LICENSE"}

  # ――― Author Metadata  ――――――――――――――――――――――――――――――――――――――――――――――――――――――――― #

  s.authors                = {"Preston Jennings" => "prestonj@bitheads.com"}
  s.social_media_url       = "https://twitter.com/braincloudbaas"

  # ――― Source Location ―――――――――――――――――――――――――――――――――――――――――――――――――――――――――― #

  s.source                 = {:git => "https://github.com/getbraincloud/braincloud-cpp.git", :tag => s.version}

  # ――― Source Code ―――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――― #

#  s.header_mappings_dir    = 'Shared'
#  s.public_header_files    = 'Shared/**/*.{h,hh}'
  #s.source_files           = 'Shared/**/*.{c,cpp,h,hh,mm}'
#  s.header_mappings_dir     = "include"
  s.public_header_files     = "../../include/braincloud/*.h"
  s.private_header_files    = "../../include/braincloud/internal/*.h", "../../include/braincloud/internal/mac/*.h"

  s.source_files            = "../../src/*.{c,cpp}", "../../src/mac/*.{c,cpp,mm}"
  s.ios.vendored_library    = "../../lib/curl_apple/curl/iOS/*.a", "../../lib/curl_apple/openssl/iOS/*.a"
  s.osx.vendored_library    = "../../lib/curl_apple/curl/OSX/*.a", "../../lib/curl_apple/openssl/OSX/*.a"
  s.tvos.vendored_library   = "../../lib/curl_apple/curl/tvOS/*.a", "../../lib/curl_apple/openssl/tvOS/*.a"

  # ――― Project Linking ―――――――――――――――――――――――――――――――――――――――――――――――――――――――――― #

  s.libraries              = 'c++', 'z'
  s.osx.framework          = 'LDAP'

end
