# Build izenelib
[
 ENV["EXTRA_CMAKE_MODULES_DIRS"],
 File.join(File.dirname(__FILE__), "../cmake"), # in same top directory
 File.join(File.dirname(__FILE__), "../../cmake--master/workspace") # for hudson
].each do |dir|
  next unless dir
  dir = File.expand_path(dir)
  if File.exists? File.join(dir, "Findizenelib.cmake")
    $: << File.join(dir, "lib")
    ENV["EXTRA_CMAKE_MODULES_DIRS"] = dir
  end
end

require "izenesoft/tasks"

task :default => :cmake

IZENESOFT::CMake.new do |t|
  t.source_dir = "."
end

IZENESOFT::BoostTest.new do |t|
  t.timeout = 1800
end
task "test_clobber" do
  Dir["testbin/*.{dat,bak,xml}"].each do |f|
    rm_rf f
  end
end

task :env do
  sh "/usr/bin/env"
end
