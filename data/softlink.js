function goTo(uri) {
  var inputs = document.getElementsByTagName("INPUT");
  for (var i = 0; i < inputs.length; i++)
    inputs[i].disabled = true;

  var form = document.forms[0];
  form.action = uri;
  form.submit();
}