/** @format */

const fs = require("fs");
console.log("Converting index.html");
var file = fs.readFileSync("source/data/index.html").toString();

if (!fs.existsSync("source/media")) fs.mkdirSync("source/media");

file = file.replaceAll("\t", " ");
file = file.replaceAll("\r\n", "");
file = file.replaceAll("  ", "");
fs.writeFileSync(
    "source/media/index.html.h",
    `const char PROGMEM mainPage[] = ${JSON.stringify(file)};`
);

console.log("Converting cssupload.html")
file = fs.readFileSync("source/data/cssupload.html").toString();

file = file.replaceAll("\t", " ");
file = file.replaceAll("\r\n", "");
file = file.replaceAll("  ", "");
fs.writeFileSync(
    "source/media/cssupload.html.h",
    `const char PROGMEM cssUploadPage[] = ${JSON.stringify(file)};`
);

console.log("Converting style.css");
var cssPurge = require("css-purge");
var cssFile = fs.readFileSync("source/data/style.css").toString();

cssPurge.purgeCSS(
    cssFile,
    {
        trim: true,
        shorten: true,
    },
    function (error, result) {
        if (error) console.log(error);
        else
            fs.writeFileSync(
                "source/media/style.css.h",
                `const char PROGMEM css[] = ${JSON.stringify(result)};`
            );
    }
);

(async () => {
    console.log("Converting script.js");
    const { minify_sync } = require("terser");
    var jsFile = fs.readFileSync("source/data/script.js").toString();
    jsFile = minify_sync(jsFile, {
        mangle: false,
    }).code;

    fs.writeFileSync(
        "source/media/script.js.h",
        `const char PROGMEM scriptJs[] = ${JSON.stringify(jsFile)};`
    );

    console.log("Converting favicon.ico");

    var favicon = fs.readFileSync("source/data/favicon.svg").toString();
    fs.writeFileSync(
        "source/media/favicon.svg.h",
        `const char PROGMEM favicon[] = ${JSON.stringify(favicon)};`
    );

    console.log("Incrementing version number");

    var versionFile = fs.readFileSync("source/firmVer.h").toString();

    var versionNum = versionFile
        .split('#define FIRMWARE_VERSION "v')[1]
        .split('"')[0];

    console.log(`Old firmware ver: v${versionNum}`);
    var major = versionNum.split(".")[0];
    var minor = versionNum.split(".")[1];
    var patch = versionNum.split(".")[2];

    if (!major || !minor || !patch) {
        console.log("Invalid version num");
    } else {
        patch++;
        console.log(`New firmvare ver: v${major}.${minor}.${patch}`);
				fs.writeFileSync("source/firmVer.h", `#define FIRMWARE_VERSION "v${major}.${minor}.${patch}"`);
    };

		if(fs.existsSync("./bin") && fs.existsSync(".pio/build/esp12/firmware.bin")){
			fs.copyFileSync(".pio/build/esp12/firmware.bin", `./bin/v${versionNum}.bin`);
		}
})();
