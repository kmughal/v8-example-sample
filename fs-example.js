log('Number of cors in your computer :' + cors)

readFile(
  '/Users/khurram/Documents/Code/source-codes/depot_tools/v8/samples/sample.cc',
  (err, fileContents) => {
    if (err) {
      log('Something went wrong : ' + err)
      return
    }
   //log(fileContents);
    helloWorld().then(v => log(v));
  }
)

