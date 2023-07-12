#define HTML_INDEX "<html>\n    <body>\n        <div id=\"display\">hello, world</div>\n        <script>\n            function update(){\n                let req = new XMLHttpRequest()\n                let taken = false\n                req.onreadystatechange = function(){\n                    if(req.readyState != 4)\n                        return\n                    if(taken)\n                        return\n                    taken = true\n                    document.getElementById(\"display\").innerText = req.responseText\n                    setTimeout(update, 100)\n                }\n                req.open(\"GET\", \"/data\")\n                req.send()\n            }\n\n            update();\n        </script>\n    </body>\n</html>"
