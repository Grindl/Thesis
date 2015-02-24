/*\
|*|
|*|  Base64 / binary data / UTF-8 strings utilities
|*|
|*|  https://developer.mozilla.org/en-US/docs/Web/JavaScript/Base64_encoding_and_decoding
|*|
\*/

/* Array of bytes to base64 string decoding */

function b64ToUint6(nChr) {

    return nChr > 64 && nChr < 91 ?
      nChr - 65
    : nChr > 96 && nChr < 123 ?
      nChr - 71
    : nChr > 47 && nChr < 58 ?
      nChr + 4
    : nChr === 43 ?
      62
    : nChr === 47 ?
      63
    :
      0;

}

function base64DecToArr(sBase64, nBlocksSize) {

    var 
    sB64Enc = sBase64.replace(/[^A-Za-z0-9\+\/]/g, ""), nInLen = sB64Enc.length,
    nOutLen = nBlocksSize ? Math.ceil((nInLen * 3 + 1 >> 2) / nBlocksSize) * nBlocksSize : nInLen * 3 + 1 >> 2, taBytes = new Uint8Array(nOutLen);

    for (var nMod3, nMod4, nUint24 = 0, nOutIdx = 0, nInIdx = 0; nInIdx < nInLen; nInIdx++) {
        nMod4 = nInIdx & 3;
        nUint24 |= b64ToUint6(sB64Enc.charCodeAt(nInIdx)) << 18 - 6 * nMod4;
        if (nMod4 === 3 || nInLen - nInIdx === 1) {
            for (nMod3 = 0; nMod3 < 3 && nOutIdx < nOutLen; nMod3++, nOutIdx++) {
                taBytes[nOutIdx] = nUint24 >>> (16 >>> nMod3 & 24) & 255;
            }
            nUint24 = 0;

        }
    }

    return taBytes;
}

/* Base64 string to array encoding */

function uint6ToB64(nUint6) {

    return nUint6 < 26 ?
      nUint6 + 65
    : nUint6 < 52 ?
      nUint6 + 71
    : nUint6 < 62 ?
      nUint6 - 4
    : nUint6 === 62 ?
      43
    : nUint6 === 63 ?
      47
    :
      65;

}

function base64EncArr(aBytes) {

    var nMod3 = 2, sB64Enc = "";

    for (var nLen = aBytes.length, nUint24 = 0, nIdx = 0; nIdx < nLen; nIdx++) {
        nMod3 = nIdx % 3;
        if (nIdx > 0 && (nIdx * 4 / 3) % 76 === 0) { sB64Enc += "\r\n"; }
        nUint24 |= aBytes[nIdx] << (16 >>> nMod3 & 24);
        if (nMod3 === 2 || aBytes.length - nIdx === 1) {
            sB64Enc += String.fromCharCode(uint6ToB64(nUint24 >>> 18 & 63), uint6ToB64(nUint24 >>> 12 & 63), uint6ToB64(nUint24 >>> 6 & 63), uint6ToB64(nUint24 & 63));
            nUint24 = 0;
        }
    }

    return sB64Enc.substr(0, sB64Enc.length - 2 + nMod3) + (nMod3 === 2 ? '' : nMod3 === 1 ? '=' : '==');

}

/* UTF-8 array to DOMString and vice versa */

function UTF8ArrToStr(aBytes) {

    var sView = "";

    for (var nPart, nLen = aBytes.length, nIdx = 0; nIdx < nLen; nIdx++) {
        nPart = aBytes[nIdx];
        sView += String.fromCharCode(
      nPart > 251 && nPart < 254 && nIdx + 5 < nLen ? /* six bytes */
        /* (nPart - 252 << 30) may be not so safe in ECMAScript! So...: */
        (nPart - 252) * 1073741824 + (aBytes[++nIdx] - 128 << 24) + (aBytes[++nIdx] - 128 << 18) + (aBytes[++nIdx] - 128 << 12) + (aBytes[++nIdx] - 128 << 6) + aBytes[++nIdx] - 128
      : nPart > 247 && nPart < 252 && nIdx + 4 < nLen ? /* five bytes */
        (nPart - 248 << 24) + (aBytes[++nIdx] - 128 << 18) + (aBytes[++nIdx] - 128 << 12) + (aBytes[++nIdx] - 128 << 6) + aBytes[++nIdx] - 128
      : nPart > 239 && nPart < 248 && nIdx + 3 < nLen ? /* four bytes */
        (nPart - 240 << 18) + (aBytes[++nIdx] - 128 << 12) + (aBytes[++nIdx] - 128 << 6) + aBytes[++nIdx] - 128
      : nPart > 223 && nPart < 240 && nIdx + 2 < nLen ? /* three bytes */
        (nPart - 224 << 12) + (aBytes[++nIdx] - 128 << 6) + aBytes[++nIdx] - 128
      : nPart > 191 && nPart < 224 && nIdx + 1 < nLen ? /* two bytes */
        (nPart - 192 << 6) + aBytes[++nIdx] - 128
      : /* nPart < 127 ? */ /* one byte */
        nPart
    );
    }

    return sView;

}

function strToUTF8Arr(sDOMStr) {

    var aBytes, nChr, nStrLen = sDOMStr.length, nArrLen = 0;

    /* mapping... */

    for (var nMapIdx = 0; nMapIdx < nStrLen; nMapIdx++) {
        nChr = sDOMStr.charCodeAt(nMapIdx);
        nArrLen += nChr < 0x80 ? 1 : nChr < 0x800 ? 2 : nChr < 0x10000 ? 3 : nChr < 0x200000 ? 4 : nChr < 0x4000000 ? 5 : 6;
    }

    aBytes = new Uint8Array(nArrLen);

    /* transcription... */

    for (var nIdx = 0, nChrIdx = 0; nIdx < nArrLen; nChrIdx++) {
        nChr = sDOMStr.charCodeAt(nChrIdx);
        if (nChr < 128) {
            /* one byte */
            aBytes[nIdx++] = nChr;
        } else if (nChr < 0x800) {
            /* two bytes */
            aBytes[nIdx++] = 192 + (nChr >>> 6);
            aBytes[nIdx++] = 128 + (nChr & 63);
        } else if (nChr < 0x10000) {
            /* three bytes */
            aBytes[nIdx++] = 224 + (nChr >>> 12);
            aBytes[nIdx++] = 128 + (nChr >>> 6 & 63);
            aBytes[nIdx++] = 128 + (nChr & 63);
        } else if (nChr < 0x200000) {
            /* four bytes */
            aBytes[nIdx++] = 240 + (nChr >>> 18);
            aBytes[nIdx++] = 128 + (nChr >>> 12 & 63);
            aBytes[nIdx++] = 128 + (nChr >>> 6 & 63);
            aBytes[nIdx++] = 128 + (nChr & 63);
        } else if (nChr < 0x4000000) {
            /* five bytes */
            aBytes[nIdx++] = 248 + (nChr >>> 24);
            aBytes[nIdx++] = 128 + (nChr >>> 18 & 63);
            aBytes[nIdx++] = 128 + (nChr >>> 12 & 63);
            aBytes[nIdx++] = 128 + (nChr >>> 6 & 63);
            aBytes[nIdx++] = 128 + (nChr & 63);
        } else /* if (nChr <= 0x7fffffff) */{
            /* six bytes */
            aBytes[nIdx++] = 252 + (nChr >>> 30);
            aBytes[nIdx++] = 128 + (nChr >>> 24 & 63);
            aBytes[nIdx++] = 128 + (nChr >>> 18 & 63);
            aBytes[nIdx++] = 128 + (nChr >>> 12 & 63);
            aBytes[nIdx++] = 128 + (nChr >>> 6 & 63);
            aBytes[nIdx++] = 128 + (nChr & 63);
        }
    }

    return aBytes;

}

//End Mozilla library

//Begin Jeff Listman's javascript client

//var serverAddress = request.serverAddress().toString();// inserted beforehand
window.URL = window.URL || window.webkitURL;
var canvas = document.getElementById("CanvasStream");
var canvasContext = canvas.getContext('2d');
var currentFrameImageData = canvasContext.createImageData(640, 480);
for (var i = 0; i < currentFrameImageData.data.length; i++) {
currentFrameImageData.data[i] = 255;//to make sure we have something
};
canvasContext.putImageData(currentFrameImageData, 0, 0);
//canvas.style.display = "none";
var renderTarget = document.getElementById("RenderTarget");
var renderContext = renderTarget.getContext("2d");

var profiler = document.createElement('div');
document.body.appendChild(profiler);
var TimeOfLastFrame = window.performance.now();
var totalTimeElapsed = 5; //HACK this is based on the assumption that 5 seconds of data are sent to video before playing
var timeSpentProcessingSinceLastProfilerUpdate = 0;
var numFramesSinceLastProfilerUpdate = 0;
var bandwidthSinceLastProfilerUpdate = 0;
var image = document.getElementById("ImageStream");

var video = document.getElementById("VideoStream");
var streamedVideoFile = new Blob([], {type : "video/ogg"});

var streamedVideoURL = window.URL.createObjectURL(streamedVideoFile);
var existingVideo = new Uint8Array();

window.URL.revokeObjectURL = window.URL.revokeObjectURL || window.webkitURL.revokeObjectURL;

var keyEvents = new Uint8Array(256);
var numKeyEventsSinceLastSend = 0;

function WebSocketTest() {
    if ("WebSocket" in window) {
        var ws = new WebSocket("ws://" + serverAddress + "/ws");
        ws.binaryType = 'arraybuffer';
        ws.onopen = function () {
            //ws.send("Hello, world!");
            TimeOfLastFrame = window.performance.now();
            updatePerformance();
        };
        ws.onmessage = function (evt) {

            keyEvents[numKeyEventsSinceLastSend * 2] = 0; //null terminating the key events
            ws.send(keyEvents);
            numKeyEventsSinceLastSend = 0;

            var startTime = window.performance.now();
            var TimeSinceLastFrame = window.performance.now() - TimeOfLastFrame;
            totalTimeElapsed += TimeSinceLastFrame;
            TimeOfLastFrame = window.performance.now();
            var msg = evt.data;

            numFramesSinceLastProfilerUpdate++;
            bandwidthSinceLastProfilerUpdate += msg.byteLength;


            if (compressionType == "NONE") {
                var currentFrameImageData = canvasContext.createImageData(640, 480);
                var frameAsBytes = new Uint8Array(msg);
                for (var i = 0; i < frameAsBytes.length; i++) {
                    currentFrameImageData.data[i] = frameAsBytes[i];
                };
                canvasContext.putImageData(currentFrameImageData, 0, 0);
                renderContext.save();
                renderContext.scale(1, -1);
                renderContext.drawImage(canvas, 0, -480);
                renderContext.restore();

            }
            else if (compressionType == "RLE") {
                var frameAsBytes = new Uint8Array(msg);
                //var frameAsInts = new Uint32Array(frameAsBytes, 0, 8);
                var greenChannelStart = frameAsBytes[0] + 256 * frameAsBytes[1] + 256 * 256 * frameAsBytes[2] + 256 * 256 * 256 * frameAsBytes[3]; //ugly way of dealing with endianness and casting
                var blueChannelStart = frameAsBytes[4] + 256 * frameAsBytes[5] + 256 * 256 * frameAsBytes[6] + 256 * 256 * 256 * frameAsBytes[7];
                var currentFrameImageData = canvasContext.createImageData(640, 480);
                for (var i = 0; i < currentFrameImageData.data.length; i++) {
                    currentFrameImageData.data[i] = 255; //to make sure we have something
                };
                var pixelMask = 0xf8;
                var runLengthMask = 0x7;
                var imageIndex = 0;
                for (var frameIndex = 8; frameIndex < frameAsBytes.length; frameIndex++) {
                    var runLength = frameAsBytes[frameIndex] & runLengthMask;
                    var pixelData = frameAsBytes[frameIndex] & pixelMask;
                    if (runLength == 7) {
                        frameIndex++;
                        runLength = frameAsBytes[frameIndex];
                    }
                    for (var i = 0; i < runLength; i++) {
                        currentFrameImageData.data[imageIndex] = pixelData;
                        imageIndex += 4;
                    }
                    if (frameIndex == greenChannelStart - 1) {
                        imageIndex = 1;
                    }
                    else if (frameIndex == blueChannelStart - 1) {
                        imageIndex = 2;
                    }
                }
                canvasContext.putImageData(currentFrameImageData, 0, 0);
                renderContext.save();
                renderContext.scale(1, -1);
                renderContext.drawImage(canvas, 0, -480);
                renderContext.restore();
            }
            else if (compressionType == "JPEG") {
                var frameAsBytes = new Uint8Array(msg);
                /*image.src = 'data:image/jpeg;base64,' + base64EncArr(frameAsBytes);
                image.style.display = "none";*/
                var frameAsBlob = new Blob([frameAsBytes], { type: "image/jpeg" });
                var imageURL = window.URL.createObjectURL(frameAsBlob);
                image.src = imageURL;
                //image.style.display = "none";*/
                //NOTE: this is dramatically faster on the client side (~2 orders of magnitude) but it doesn't interact properly with the canvas
                renderContext.save();
                renderContext.scale(1, -1);
                renderContext.drawImage(image, 0, -480);
                renderContext.restore();
            }
            else if (compressionType == "PNG") {
                var frameAsBytes = new Uint8Array(msg);
                /*image.src = 'data:image/png;base64,' + base64EncArr(frameAsBytes);
                image.style.display = "none";*/
                var frameAsBlob = new Blob([frameAsBytes], { type: "image/png" });
                var imageURL = window.URL.createObjectURL(frameAsBlob);
                image.src = imageURL;
                //image.style.display = "none";*/
                //NOTE: this is dramatically faster on the client side (~2 orders of magnitude) but it doesn't interact properly with the canvas
                renderContext.save();
                renderContext.scale(1, -1);
                renderContext.drawImage(image, 0, -480);
                renderContext.restore();
            }
            else if (compressionType == "THEORA") {
                var frameAsBytes = new Uint8Array(msg);

                window.URL.revokeObjectURL(video.src); //frees up the URL to prevent pollution
                var newVideoFile = new Blob([streamedVideoFile, frameAsBytes], { type: "video/ogg" });
                window.URL.revokeObjectURL(streamedVideoFile)
                streamedVideoFile = newVideoFile;
                streamedVideoURL = window.URL.createObjectURL(streamedVideoFile);
                video.src = streamedVideoURL;
                video.currentTime = totalTimeElapsed;

                //video.src = 'data:video/ogg;base64,' + base64EncArr(frameAsBytes);

                /*var compositeArray = new Uint8Array(frameAsBytes.byteLength + existingVideo.byteLength);
                compositeArray.set(existingVideo, 0);
                compositeArray.set(frameAsBytes, existingVideo.byteLength);
                existingVideo = compositeArray;
                video.src = 'data:video/ogg;base64,' + base64EncArr(existingVideo);*/
            }
            timeSpentProcessingSinceLastProfilerUpdate += (window.performance.now() - startTime);
            /*profiler.innerHTML = "Time Elapsed: " + ((window.performance.now() - startTime) * .001) + " s"
            + "</br>Time Since Last Frame: " + (TimeSinceLastFrame * .001) + " s"
            + "</br>Bandwidth: " + (msg.byteLength / TimeSinceLastFrame) + " KBps";*/
        };
        ws.onclose = function () {
            alert("WebSocket closed.");
        };
    }
    else {
        alert("This browser does not support WebSockets.");
    }
}



var updatePerformance = function () {
    //ws.send("Hello, world!");
    profiler.innerHTML = "Time Spent Processing: " + (timeSpentProcessingSinceLastProfilerUpdate * 0.1) + " %"
                + "</br>Frames Per Second: " + numFramesSinceLastProfilerUpdate + ""
                + "</br>Bandwidth: " + (bandwidthSinceLastProfilerUpdate/1000000.0) + " MiBps";
    numFramesSinceLastProfilerUpdate = 0;
    bandwidthSinceLastProfilerUpdate = 0;
    timeSpentProcessingSinceLastProfilerUpdate = 0;
    setTimeout(updatePerformance, 1000);
}


var onKeyDown = function (keyPressed) {
    var eventIndex = numKeyEventsSinceLastSend * 2;
    keyEvents[eventIndex] = keyPressed.keyCode;
    keyEvents[eventIndex + 1] = 1;
    numKeyEventsSinceLastSend++;
}

var onKeyUp = function (keyPressed) {
    var eventIndex = numKeyEventsSinceLastSend * 2;
    keyEvents[eventIndex] = keyPressed.keyCode;
    keyEvents[eventIndex + 1] = 0;
    numKeyEventsSinceLastSend++;
}

window.addEventListener('keydown', onKeyDown);

window.addEventListener('keyup', onKeyUp);