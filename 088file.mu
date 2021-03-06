# Wrappers around file system primitives that take a 'filesystem' object and
# are thus easier to test.

container filesystem [
  data:address:array:file-mapping
]

container file-mapping [
  name:text
  contents:text
]

def start-reading fs:address:filesystem, filename:text -> contents:address:source:character [
  local-scope
  load-ingredients
  {
    break-if fs
    # real file system
    file:number <- $open-file-for-reading filename
    assert file, [file not found]
    contents:address:source:character, sink:address:sink:character <- new-channel 30
    start-running transmit-from-file file, sink
    return
  }
  # fake file system
  i:number <- copy 0
  data:address:array:file-mapping <- get *fs, data:offset
  len:number <- length *data
  {
    done?:boolean <- greater-or-equal i, len
    break-if done?
    tmp:file-mapping <- index *data, i
    i <- add i, 1
    curr-filename:text <- get tmp, name:offset
    found?:boolean <- equal filename, curr-filename
    loop-unless found?
    contents:address:source:character, sink:address:sink:character <- new-channel 30
    curr-contents:text <- get tmp, contents:offset
    start-running transmit-from-text curr-contents, sink
    return
  }
  return 0/not-found
]

def transmit-from-file file:number, sink:address:sink:character -> sink:address:sink:character [
  local-scope
  load-ingredients
  {
    c:character, eof?:boolean <- $read-from-file file
    break-if eof?
    sink <- write sink, c
    loop
  }
  sink <- close sink
  file <- $close-file file
]

def transmit-from-text contents:text, sink:address:sink:character -> sink:address:sink:character [
  local-scope
  load-ingredients
  i:number <- copy 0
  len:number <- length *contents
  {
    done?:boolean <- greater-or-equal i, len
    break-if done?
    c:character <- index *contents, i
    sink <- write sink, c
    i <- add i, 1
    loop
  }
  sink <- close sink
]

def start-writing fs:address:filesystem, filename:text -> sink:address:sink:character, routine-id:number [
  local-scope
  load-ingredients
  source:address:source:character, sink:address:sink:character <- new-channel 30
  {
    break-if fs
    # real file system
    file:number <- $open-file-for-writing filename
    assert file, [no such file]
    routine-id <- start-running transmit-to-file file, source
    reply
  }
  # fake file system
  # beware: doesn't support multiple concurrent writes yet
  routine-id <- start-running transmit-to-fake-file fs, filename, source
]

def transmit-to-file file:number, source:address:source:character -> source:address:source:character [
  local-scope
  load-ingredients
  {
    c:character, done?:boolean, source <- read source
    break-if done?
    $write-to-file file, c
    loop
  }
  file <- $close-file file
]

def transmit-to-fake-file fs:address:filesystem, filename:text, source:address:source:character -> fs:address:filesystem, source:address:source:character [
  local-scope
  load-ingredients
  # compute new file contents
  buf:address:buffer <- new-buffer 30
  {
    c:character, done?:boolean, source <- read source
    break-if done?
    buf <- append buf, c
    loop
  }
  contents:text <- buffer-to-array buf
  new-file-mapping:file-mapping <- merge filename, contents
  # write to filesystem
  curr-filename:text <- copy 0
  data:address:array:file-mapping <- get *fs, data:offset
  # replace file contents if it already exists
  i:number <- copy 0
  len:number <- length *data
  {
    done?:boolean <- greater-or-equal i, len
    break-if done?
    tmp:file-mapping <- index *data, i
    curr-filename <- get tmp, name:offset
    found?:boolean <- equal filename, curr-filename
    loop-unless found?
    put-index *data, i, new-file-mapping
    reply
  }
  # if file didn't already exist, make room for it
  new-len:number <- add len, 1
  new-data:address:array:file-mapping <- new file-mapping:type, new-len
  put *fs, data:offset, new-data
  # copy over old files
  i:number <- copy 0
  {
    done?:boolean <- greater-or-equal i, len
    break-if done?
    tmp:file-mapping <- index *data, i
    put-index *new-data, i, tmp
  }
  # write new file
  put-index *new-data, len, new-file-mapping
]
