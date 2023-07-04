window.onload = async function () {
  const ssid = document.getElementById("ssid");
  const passphrase = document.getElementById("passphrase");

  document.querySelector("form").addEventListener("submit", async function (e) {
    e.preventDefault()
    try {
      let body = {}
      document.querySelectorAll("input[type='text']").forEach((elt) => {
        body[elt.id] = elt.value;
      })
      let resp = await fetch("pid.json", {
        "method": "PUT",
        "body": JSON.stringify(body),
      });
    } catch (error) {
      console.log(error);
    }
  });

  try {
    let res = await fetch("pid.json");
    let body = await res.json();
    document.querySelectorAll("input[type='text']").forEach((elt) => {
      elt.value = body[elt.id];
    })
  } catch (error) {
    console.log(error);
  }
}
