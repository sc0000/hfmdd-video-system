const axios = require('axios');
const https = require('https');
const nodemailer = require('nodemailer');

// set to true for additional logging
const debugMode = false;

const sendMailFromTransport = 
(host, port, user, pass, sender, receiver, subject, text) => {
  const transport = nodemailer.createTransport({
    host: host,
    // port: port,
    secure: true, 
    auth: {
      user: user,
      pass: pass
    }
  });
  
  const mailOptions = {
    from: sender,
    to: receiver,
    subject: subject,
    text: text
  };

  transport.sendMail(mailOptions, (error, info) => {
    if (error) console.log(error);
    else console.log("Email sent to " + receiver + ".\nIf you don't find it in your in-box, " +
      "please check your spam folder as well!");
  });
}

const httpsAgent = new https.Agent({
  rejectUnauthorized: false,
  
  // ! This disables client verification. Setup a renewed certificate! 
})

const readInput = () => {
  return new Promise((resolve, reject) => {
    let inputData = '';

    process.stdin.setEncoding('utf8');

    process.stdin.on('data', (input) => {
      inputData += input;
    });

    process.stdin.on('end', () => {
      try {
        const parsedInput = JSON.parse(inputData);
        resolve(parsedInput);
      } catch (error) {
        reject(error);
      }
    })
  });
}

const sendFiles = async (basePath, userPath, receiver, subject, nasIP, nasPort, 
  nasUsername, nasPassword, mailHost, mailPort, mailUser, mailPassword, 
  mailSenderAddress, mailBody) => {  
  try {
    const path = basePath + userPath;

    const apiInfoEndpoint = `https://${nasIP}:${nasPort}/webapi/entry.cgi?api=SYNO.API.Info&version=1&method=query&query=SYNO.API.Auth,SYNO.SynologyDrive.`;
    const apiInfoResponse = await axios.get(apiInfoEndpoint, { httpsAgent });

    if (!apiInfoResponse.data.success) {
      if (debugMode) console.log("Getting API info failed. Error: ", apiInfoResponse.data.error);
      return;
    }

    if (debugMode) console.log("Get API info --Done");
    if (debugMode) console.log(apiInfoResponse.data);

    const loginEndPoint = `https://${nasIP}:${nasPort}/webapi/entry.cgi?api=SYNO.API.Auth&version=6&method=login&account=${nasUsername}&passwd=${nasPassword}&enable_syno_token=no`;
    const loginResponse = await axios.get(loginEndPoint, { httpsAgent });

    if (!loginResponse.data.success) {
      if (debugMode) console.log("Login failed. Error: ", loginResponse.data.error);
      return;
    }

    if (debugMode) console.log("Login --Done");

    const teamFoldersEndPoint = `https://${nasIP}:${nasPort}/webapi/entry.cgi?api=SYNO.SynologyDrive.TeamFolders&version=1&method=list&sort_direction=asc&sort_by=owner&offset=0&limit=1000&_sid=${loginResponse.data.data.sid}`;
    const teamFoldersResponse = await axios.get(teamFoldersEndPoint, { httpsAgent });

    if (!teamFoldersResponse.data.success) {
      if (debugMode) console.log("SynologyDrive API access failed. Error: ", teamFoldersResponse.data.error);
      return;
    }

    if (debugMode) console.log(teamFoldersResponse.data.data.items[1].file_id);

    const filesEndPoint = `https://${nasIP}:${nasPort}/webapi/entry.cgi?api=SYNO.SynologyDrive.Files&version=3&method=list&path=${path}&_sid=${loginResponse.data.data.sid}`;
    const filesResponse = await axios.get(filesEndPoint, { httpsAgent });

    if (!filesResponse.data.success) {
      console.log("SynologyDrive API access failed. Error: ", filesResponse.data.error);
      return;
    }

    if (debugMode) filesResponse.data.data.items.forEach(i => console.log(i));

    const shareEndPoint = `https://${nasIP}:${nasPort}/webapi/entry.cgi?api=SYNO.SynologyDrive.AdvanceSharing&version=1&method=create&path=${path}&_sid=${loginResponse.data.data.sid}`;
    const shareResponse = await axios.get(shareEndPoint, { httpsAgent });

    if (!shareResponse.data.success) {
      if (debugMode) console.log("SynologyDrive sharing failed. Error: ", shareResponse.data.error);
      return;
    }

    const url = shareResponse.data.data.url;
    const text = mailBody + `${url}`;

    // Send URL to output
    // console.log(url);

    sendMailFromTransport(mailHost, mailPort, mailUser, mailPassword, 
      mailSenderAddress, receiver, subject, text);

    const logoutEndPoint = `https://${nasIP}:${nasPort}/webapi/entry.cgi?api=SYNO.API.Auth&version=6&method=logout&_sid=${loginResponse.data.data.sid}`;
    const logoutResponse = await axios.get(logoutEndPoint, { httpsAgent });
    
    if (!logoutResponse.data.success) {
      if (debugMode) console.log("Logout failed. Error: ", logoutResponse.data.error);
      return;
    }

    if (debugMode) console.log("Logout --Done");

  } catch (error) {
    if (debugMode) console.error('Error during login:', error);
  }
}

const sendWarning = async (mailHost, mailPort, mailUser, 
  mailPassword, mailSenderAddress, receiver, subject, body) => {
  sendMailFromTransport(mailHost, mailPort, mailUser, mailPassword, 
    mailSenderAddress, receiver, subject, body);
}

const sendAdminMail = async(mailHost, mailPort, mailUser, 
  mailPassword, mailSenderAddress, receiver, subject, body) => {
  sendMailFromTransport(mailHost, mailPort, mailUser, 
    mailPassword, mailSenderAddress, receiver, subject, body);
}

const sendMail = async () => {
  const input = await readInput();

  const type = input.type;

  const mailHost = input.mailHost.toString();
  const mailPort = input.mailPort;
  const mailUser = input.mailUser.toString();
  const mailPassword = input.mailPassword.toString();
  const mailSenderAddress = input.mailSenderAddress.toString();

  const receiver = input.receiver;
  const subject = input.subject;
  const mailBody = input.mailBody;

  const nasIP = input.nasIP;
  const nasPort = input.nasPort;
  const nasUsername = input.nasUser;
  const nasPassword = input.nasPassword;

  if (type == 'send-files') {
    const basePath = input.basePath;
    const userPath = input.path;

    sendFiles(basePath, userPath, receiver, subject, nasIP, nasPort, nasUsername, 
      nasPassword, mailHost, mailPort, mailUser, mailPassword, mailSenderAddress, mailBody);
    return;
  }
    
  if (type == 'booking-conflict-warning') {
    sendWarning(mailHost, mailPort, mailUser, mailPassword, mailSenderAddress, 
      receiver, subject, mailBody);
    return;
  }

  if (type == 'admin-email') {
    sendAdminMail(mailHost, mailPort, mailUser, mailPassword, mailSenderAddress, 
      receiver, subject, mailBody);
    return;
  }
    
  console.log("No mail sent. Invalid type?");
}

sendMail();
