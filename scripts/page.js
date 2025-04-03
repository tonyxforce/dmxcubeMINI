/** @format */

const fs = require("fs");
console.log("Converting index.html");
var file = fs.readFileSync("source/data/index.html").toString();

file = file.replaceAll("\t", " ");
file = file.replaceAll("\r\n", "");
file = file.replaceAll("  ", "");
fs.writeFileSync(
    "source/media/index.html.h",
    `const char PROGMEM mainPage[] = ${JSON.stringify(file)};`
);

console.log("Converting style.css");
var cssPurge = require("css-purge");
var cssFile = fs.readFileSync("source/data/style2.css").toString();

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

console.log("Converting script.js");
(async () => {
	const { minify_sync } = require("terser");

    var jsFile = minify_sync("source/data/script.js", {
			mangle:{
				reserved: ["menuClick"]
			}
		}).code;

    fs.writeFileSync(
        "source/media/script.js.h",
        `const char PROGMEM scriptJs[] = ${JSON.stringify(jsFile)};`
    );
})();
