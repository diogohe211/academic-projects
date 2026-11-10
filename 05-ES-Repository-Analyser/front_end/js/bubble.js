document.addEventListener("DOMContentLoaded", function() {
    const width = window.innerWidth - window.innerWidth/4; 
    const height = window.innerHeight - 100;
    const format = d3.format(",d");
    let currentCharacteristic = "filesize";
    let currentData = null;
    let fileHistoryData = null;
    let currentPath = [];

    const bubble = d3.pack()
        .size([width-20, height-20]) //padding so the bubbles dont appear clipped
        .padding(5);

    const svg = d3.select("#bubble-chart")
        .append("svg")
        .attr("width", width)
        .attr("height", height)
        .attr("class", "bubble");

    const g = svg.append("g");

    const zoom = d3.zoom()
        .scaleExtent([0.5, 20])
        .on("zoom", (event) => {
            g.attr("transform", event.transform);
        });

    svg.call(zoom);

    const backButton = document.getElementById("back-button");
    const charButtons = document.querySelectorAll(".char-button");

    const fileTypes = new Set();
    let currentTransform = d3.zoomIdentity;

    function getCharacteristicValue(metadata) {
        if (!metadata) return 0;
        const value = metadata[currentCharacteristic];
        return value ? Number(value) : 0;
    }

    function getCharacteristicLabel() {
        switch (currentCharacteristic) {
            case "filesize": return "bytes";
            case "nloc": return "linhas de código";
            case "cycComp": return "complexidade ciclomática";
            case "cogComp": return "complexidade cognitiva";
            default: return "";
        }
    }

    function getNodeColor(d) {
        if (!d.data.metadata) {
            return "#666666";
        }

        const filename = d.data.metadata.filename;
        if (!filename) {
            return "#666666";
        }

        const fileType = filename.split(".").pop().toLowerCase();
        if (!fileType || fileType === filename) {
            return "#666666";
        }

        return color(fileType);
    }

    function updateVisualization(data, fileHistoryData, path = []) {
        currentPath = path;
    
        const root = processData(data, fileHistoryData, path);
        bubble(root);
    
        // Update the folder information
        updateFolderInfo(currentData, currentPath);
    
        // Conta arquivos e pastas
        let filesWithoutData = 0;
        let hasDirectories = false;
        let hasFilesWithData = false;
    
        if (root.children) {
            root.children.forEach(child => {
                if (child.data.metadata) {
                    // É um arquivo
                    if (!child.data.metadata[currentCharacteristic] || 
                        child.data.metadata[currentCharacteristic] === "N/A" ||
                        child.data.metadata[currentCharacteristic] === null) {
                        filesWithoutData++;
                    } else {
                        hasFilesWithData = true;
                    }
                } else {
                    // É uma pasta
                    hasDirectories = true;
                }
            });
        }
    
        // Atualiza a mensagem de sem dados
        const noDataMessage = document.getElementById('no-data-message');
        const hiddenFilesCounter = document.getElementById('hidden-files-counter');
    
        // Só mostra a mensagem se não houver pastas e nem arquivos com dados
        if (!hasDirectories && !hasFilesWithData && currentCharacteristic !== "filesize") {
            noDataMessage.style.display = "block";
        } else {
            noDataMessage.style.display = "none";
        }
    
        // Mostra o contador se houver arquivos sem dados
        if (filesWithoutData > 0) {
            hiddenFilesCounter.style.display = "block";
            document.getElementById('hidden-count').textContent = filesWithoutData;
        } else {
            hiddenFilesCounter.style.display = "none";
        }
    
        g.selectAll(".node").remove();
    
        const nodes = g.selectAll(".node")
            .data(root.children || [], d => d.data.name)
            .enter()
            .append("g")
            .attr("class", "node")
            .attr("transform", d => `translate(${d.x},${d.y})`);
    
        nodes.append("circle")
            .attr("r", d => d.r)
            .style("fill", getNodeColor)
            .style("opacity", 0.8)
            .on("click", (event, d) => {
                if (d.children && d.children.length > 0) {
                    const newPath = [...currentPath, d.data.name];
                    updateVisualization(currentData, fileHistoryData, newPath);
                    centerNode(d.x, d.y);
                }
            });
    
        nodes.append("title")
            .text(d => {
                const value = d.value; 
                const label = getCharacteristicLabel();
    
                const isFile = !!d.data.metadata && !!d.data.metadata.filename;
                const author = isFile ? d.data.metadata.author : null;
                const commitDate = isFile ? d.data.metadata.commitDate : "Unknown"; 
    
                return isFile
                    ? `${d.data.name}\n${format(value)} ${label}\nAuthor: ${author || "Unknown"}\nLast Update: ${commitDate || "Unknown"}`
                    : `${d.data.name}\n${format(value)} ${label}`;
            });
    
        nodes.append("text")
            .attr("dy", ".3em")
            .style("text-anchor", "middle")
            .style("font-size", d => Math.min(d.r / 3, 12) + "px")
            .style("fill", "white")
            .text(d => d.data.name);
    }
    
    

    function processData(data, fileHistoryData, path) {
        function createHierarchy(data) {
            const root = { name: "root", children: [] };
            
            for (const [filePath, metadata] of Object.entries(data)) {
                const parts = filePath.split("\\");
                let current = root;
                
                parts.forEach((part, index) => {
                    let child = current.children.find(c => c.name === part);
                    if (!child) {
                        metadata.commitDate = fileHistoryData[filePath] ? fileHistoryData[filePath].commitDate[
                            fileHistoryData[filePath].commitDate.length - 1
                        ] : null;

                        child = { 
                            name: part, 
                            children: [],
                            metadata: index === parts.length - 1 ? metadata : null
                        };
                        current.children.push(child);
                    }
                    current = child;
                });
    
                if (metadata && metadata.filename) {
                    const fileType = metadata.filename.split(".").pop().toLowerCase();
                    if (fileType && fileType !== metadata.filename) {
                        fileTypes.add(fileType);
                    }
                }
            }
            
            return root;
        }
    
        const hierarchy = createHierarchy(data);
        
        let current = hierarchy;
        for (const dir of path) {
            const next = current.children.find(c => c.name === dir);
            if (next) {
                current = next;
            } else {
                break;
            }
        }
    
        return d3.hierarchy(current)
            .sum(d => getCharacteristicValue(d.metadata))
            .sort((a, b) => b.value - a.value)
            .eachAfter(d => { 
                if (!d.data.metadata) {
                    d.value = d.children.reduce((sum, child) => sum + (child.value || 0), 0); 
                }
            })
    }
    

    function centerNode(x, y, scale = 1) {
        const transform = d3.zoomIdentity
            .translate(width / 2, height / 2)
            .scale(scale)
            .translate(-x, -y);
        
        svg.transition()
            .duration(750)
            .call(zoom.transform, transform);
        
        currentTransform = transform;
    }

    charButtons.forEach(button => {
        button.addEventListener("click", () => {
            charButtons.forEach(b => b.classList.remove("active"));
            button.classList.add("active");
            currentCharacteristic = button.dataset.char;
            
            if (currentData) {
                updateVisualization(currentData, fileHistoryData, currentPath);
            }
        });
    });

    backButton.addEventListener("click", () => {
        if (currentPath.length > 0) {
            currentPath.pop();
            updateVisualization(currentData, fileHistoryData, currentPath);
            centerNode(width / 2, height / 2);
        }
    });

    const colorScheme = [
        "#FF6B6B",
        "#4ECDC4",
        "#45B7D1",
        "#96CEB4",
        "#FFEEAD",
        "#D4A5A5",
        "#9B59B6",
        "#3498DB",
        "#E67E22",
        "#2ECC71",
        "#F1C40F",
        "#E74C3C",
        "#1ABC9C",
        "#9B59B6",
        "#34495E",
        "#16A085",
        "#27AE60",
        "#2980B9",
        "#8E44AD",
        "#F39C12"
    ];

    const color = d3.scaleOrdinal()
        .domain(Array.from(fileTypes))
        .range(colorScheme);

        function bubbleRep() {
            d3.json("http://127.0.0.1:5000/file_names")
                .then(data => {
                    currentData = data.result;
                    d3.json("http://127.0.0.1:5000/file_history")
                        .then(historyData => {
                            handleFileHistory(historyData.result);
                            updateVisualization(currentData, fileHistoryData);
                            centerNode(width / 2, height / 2);
                        })
                        .catch(error => console.error("Error loading or parsing file history data:", error));
                })
                .catch(error => console.error("Error loading or parsing file names data:", error));
        }

    function handleFileHistory(data) {
        fileHistoryData = data;
    }
    
    window.bubbleRep = bubbleRep;
    bubbleRep();
});

function updateFolderInfo(data, path) {
    let currentFolderData = {};
    let currentPath = path.join("\\");
    let folderFiles = Object.entries(data).filter(([filePath]) => filePath.startsWith(currentPath));

    // Inicializar métricas da pasta
    let totalSize = 0;
    let contributors = new Set();
    let complexity = 0;
    let lastModified = null;

    folderFiles.forEach(([filePath, fileData]) => {
        if (fileData.filesize) totalSize += fileData.filesize;
        if (fileData.author) contributors.add(fileData.author);
        if (fileData.cycComp) complexity += fileData.cycComp;
    });

    // Atualizar os elementos da interface
    document.getElementById("folder-name").textContent = path.length > 0 ? path[path.length - 1] : "Root";
    document.getElementById("folder-size").textContent = totalSize ? `${totalSize} bytes` : "N/A";
    document.getElementById("folder-complexity").textContent = complexity || "N/A";
    document.getElementById("folder-contributor").textContent = Array.from(contributors).join(", ") || "N/A";
}
