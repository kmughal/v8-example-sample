log('Number of cors in your computer :' + cors)

readFile('path of the file', (err, fileContents) => {
  if (err) {
    log('Something went wrong : ' + err)
    return
  }
  log(fileContents)
})
