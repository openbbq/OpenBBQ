window.onload = async function () {
  const ssid = document.getElementById("ssid");
  const passphrase = document.getElementById("passphrase");

  document.getElementById("wifi").addEventListener("submit", async function (e) {
    e.preventDefault()
    try {
      let resp = await fetch("wifi.json", {
        "method": "PUT",
        "body": JSON.stringify({
          "SSID": ssid.value,
          "PASSPHRASE": passphrase.value,
        }),
      });
    } catch (error) {
      console.log(error);
    }
  });

  try {
    let res = await fetch("wifi.json");
    let body = await res.json();
    ssid.value = body.SSID;
    passphrase.value = body.PASSPHRASE;
  } catch (error) {
    console.log(error);
  }
}
