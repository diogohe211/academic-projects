document.addEventListener("DOMContentLoaded", function () {
    const radioButtons = document.querySelectorAll('input[name="graph-option"]');
    const histogramContainer = document.getElementById("histogram-chart");
    const compareVariablesSection = document.getElementById('compare-variables-section');
    const variableCountSelect = document.getElementById("variable-count");
    const variablesSelectionDiv = document.getElementById("variables-selection");

        function createHistogram() {
            // Buscar dados do histórico de arquivos
            fetch('http://127.0.0.1:5000/file_history')
                .then(response => response.json())
                .then(data => {
                    const allFilesHistory = data.result;

                    // Buscar dados de nomes de arquivos
                    fetch('http://127.0.0.1:5000/file_names')  // Supondo que este seja o endpoint para os nomes de arquivos e autores
                        .then(response => response.json())
                        .then(namesData => {
                            const allFilesNames = namesData.result;

                            const files = [];
                            const complexities = [];
                            const sizes = [];
                            const linesOfCode = [];
                            const changes = [];
                            const lastCommitDates = [];
                            const authors = []; // Array para armazenar os autores

                            // Mapeando os arquivos do file_history e file_names
                            for (let filePath in allFilesHistory) {
                                if (allFilesHistory.hasOwnProperty(filePath)) {
                                    let fileData = allFilesHistory[filePath];
                                    if (fileData.cycCompHist !== null && fileData.nlocHist !== null) {
                                        files.push(filePath);
                                        complexities.push(fileData.cycCompHist[fileData.cycCompHist.length - 1] || 0);
                                        sizes.push(fileData.fileSizeHist[fileData.fileSizeHist.length - 1] || 0);
                                        linesOfCode.push(fileData.nlocHist[fileData.nlocHist.length - 1] || 0);
                                        changes.push(fileData.cycCompHist.length);
                                        lastCommitDates.push(fileData.lastCommitDate || "Unknown");

                                        // Agora buscamos o autor no file_names
                                        const author = allFilesNames[filePath] ? allFilesNames[filePath].author : "Unknown"; // Verifica o autor do arquivo
                                        authors.push(author);
                                    }
                                }
                            }

                            const sampleData = files.map((file, index) => ({file, lines: linesOfCode[index]}));
                            const Changes = files.map((file, index) => ({file, lines: changes[index]}));
                            const Complexity = files.map((file, index) => ({file, lines: complexities[index]}));
                            const Size = files.map((file, index) => ({file, lines: sizes[index]}));

                            const filteredSampleData = sampleData.filter(d => d.lines > 0);
                            const filteredChangesData = Changes.filter(d => d.lines > 0);
                            const filteredComplexityData = Complexity.filter(d => d.lines > 0);
                            const filteredSizeData = Size.filter(d => d.lines > 0);

                            function renderHistogram(data, direction = "up", yScale) {
                                
                                data.sort((a, b) => b.lines - a.lines);

                                const margin = {top: 20, right: 30, bottom: 40, left: 50};
                                const width = Math.max(80000, data.length * 40) - margin.left - margin.right;
                                const height = 400 - margin.top - margin.bottom;

                                const svg = d3
                                    .select(histogramContainer)
                                    .append("svg")
                                    .attr("width", width + margin.left + margin.right)
                                    .attr("height", height + margin.top + margin.bottom)
                                    .append("g")
                                    .attr("transform", `translate(${margin.left},${margin.top})`);

                                const x = d3
                                    .scaleBand()
                                    .domain(data.map(d => d.file))
                                    .range([0, width])
                                    .padding(0.2);

                                const y = yScale || d3
                                    .scaleLinear()
                                    .domain([0, d3.max(data, d => d.lines)])
                                    .nice()
                                    .range(direction === "up" ? [height / 2, 0] : [height / 2, height]);

                                svg
                                    .append("g")
                                    .attr("transform", `translate(0,${height / 2})`)
                                    .call(d3.axisBottom(x))
                                    .selectAll("text")
                                    .attr("transform", "rotate(-45)")
                                    .style("text-anchor", "end");

                                svg.append("g").call(d3.axisLeft(y));

                                const tooltip = d3.select("body").append("div")
                                    .attr("class", "tooltip")
                                    .style("display", "none");

                                svg
                                    .selectAll(".bar")
                                    .data(data)
                                    .enter()
                                    .append("rect")
                                    .attr("class", "bar")
                                    .attr("x", d => x(d.file))
                                    .attr("y", d => direction === "up" ? y(d.lines) : height / 2)
                                    .attr("width", x.bandwidth())
                                    .attr("height", d => direction === "up" ? height / 2 - y(d.lines) : y(d.lines) - height / 2)
                                    .attr("fill", "steelblue")
                                    .on("mouseover", function (event, d) {
                                        const fileIndex = files.indexOf(d.file); // Find the index of the file
                                        const latestCommitDate = allFilesHistory[d.file]?.commitDate?.slice(-1)[0] || "N/A"; // Get the latest commit date
                                        const author = authors[fileIndex]; // Pega o autor do arquivo

                                        const htmlContent = `
                                        <strong>File:</strong> ${d.file}<br>
                                        <strong>Lines of Code:</strong> ${linesOfCode[fileIndex]}<br>
                                        <strong>Changes:</strong> ${changes[fileIndex]}<br>
                                        <strong>Complexity:</strong> ${complexities[fileIndex]}<br>
                                        <strong>Last Commit Date:</strong> ${latestCommitDate}<br>
                                        <strong>Size:</strong> ${sizes[fileIndex]} KB<br>
                                        <strong>Author:</strong> ${author}  <!-- Aqui está a alteração -->
                                    `;
                                        tooltip.html(htmlContent)
                                            .style("display", "block")
                                            .style("left", `${event.pageX + 10}px`)
                                            .style("top", `${event.pageY + 10}px`);
                                    })
                                    .on("mousemove", function (event) {
                                        tooltip.style("left", `${event.pageX + 10}px`)
                                            .style("top", `${event.pageY + 10}px`);
                                    })
                                    .on("mouseout", function () {
                                        tooltip.style("display", "none");
                                    });
                            }

                            // Render the selected data when radio buttons are clicked
                            radioButtons.forEach(radio => {
                                radio.addEventListener("change", function () {
                                    histogramContainer.innerHTML = "";
                                    if (this.value === "lines-of-code") {
                                        renderHistogram(filteredSampleData.sort((a, b) => b.lines - a.lines));
                                    } else if (this.value === "changes") {
                                        renderHistogram(filteredChangesData.sort((a, b) => b.lines - a.lines));
                                    } else if (this.value === "complexity") {
                                        renderHistogram(filteredComplexityData.sort((a, b) => b.lines - a.lines));
                                    } else if (this.value === "size") {
                                        renderHistogram(filteredSizeData.sort((a, b) => b.lines - a.lines));
                                    
                                    } else if (this.value === "compare-variables") {
                                        renderCompareVariables();
                                    } else {
                                        histogramContainer.innerHTML = "";
                                    }
                                });
                            });

                            function renderCompareVariables() {
                                histogramContainer.innerHTML = "";

                                const selectedVariables = [];
                                const variableSelects = variablesSelectionDiv.querySelectorAll("select");

                                variableSelects.forEach(select => {
                                    const selectedValue = select.value;
                                    if (selectedValue) {
                                        selectedVariables.push({file: select.id, value: selectedValue});
                                    }
                                });

                                if (selectedVariables.length !== 2) {
                                    alert("Por favor, selecione exatamente 2 variáveis para comparar.");
                                    return;
                                }

                                // Mapeando os dados para cada variável selecionada
                                const data = selectedVariables.map(variable => {
                                    switch (variable.value) {
                                        case "lines-of-code":
                                            return files.map((file, index) => ({file, lines: linesOfCode[index]}));
                                        case "changes":
                                            return files.map((file, index) => ({file, lines: changes[index]}));
                                        case "complexity":
                                            return files.map((file, index) => ({file, lines: complexities[index]}));
                                        case "size":
                                            return files.map((file, index) => ({file, lines: sizes[index]}));
                                        default:
                                            return [];
                                    }
                                });

                                // Filtrando os dados de acordo com os valores como no exemplo
                                const filteredData1 = data[0].filter(d => {
                                    if (selectedVariables[0].value === "lines-of-code") {
                                        return d.lines > 0;  // Filtro para lines-of-code
                                    } else if (selectedVariables[0].value === "changes") {
                                        return d.lines > 0;  // Filtro para changes
                                    } else if (selectedVariables[0].value === "complexity") {
                                        return d.lines > 0;  // Filtro para complexity
                                    } else if (selectedVariables[0].value === "size") {
                                        return d.lines > 0;  // Filtro para size
                                    }
                                });

                                const filteredData2 = data[1].filter(d => {
                                    if (selectedVariables[1].value === "lines-of-code") {
                                        return d.lines > 0;  // Filtro para lines-of-code
                                    } else if (selectedVariables[1].value === "changes") {
                                        return d.lines > 0;  // Filtro para changes
                                    } else if (selectedVariables[1].value === "complexity") {
                                        return d.lines > 0;  // Filtro para complexity
                                    } else if (selectedVariables[1].value === "size") {
                                        return d.lines > 0;  // Filtro para size
                                    }
                                });

                                // Chama a função para comparar os dois conjuntos de dados filtrados
                                renderSideBySideComparison(filteredData1, filteredData2);
                            }

                            function renderSideBySideComparison(data1, data2) {
                                const margin = {top: 20, right: 30, bottom: 40, left: 50};
                                const width = Math.max(800, data1.length * 40) - margin.left - margin.right;
                                const height = 400 - margin.top - margin.bottom;

                                const svg = d3
                                    .select(histogramContainer)
                                    .append("svg")
                                    .attr("width", width + margin.left + margin.right)
                                    .attr("height", height + margin.top + margin.bottom)
                                    .append("g")
                                    .attr("transform", `translate(${margin.left},${margin.top})`);

                                const x = d3
                                    .scaleBand()
                                    .domain(data1.map(d => d.file))
                                    .range([0, width])
                                    .padding(0.1);

                                const y = d3
                                    .scaleLinear()
                                    .domain([0, d3.max([...data1, ...data2], d => d.lines)])
                                    .nice()
                                    .range([height, 0]);

                                svg
                                    .append("g")
                                    .attr("transform", `translate(0,${height})`)
                                    .call(d3.axisBottom(x))
                                    .selectAll("text")
                                    .attr("transform", "rotate(-45)")
                                    .style("text-anchor", "end");

                                svg.append("g").call(d3.axisLeft(y));

                                const tooltip = d3.select("body").append("div")
                                    .attr("class", "tooltip")
                                    .style("display", "none");

                                function showTooltip(event, d, color) {
                                    const fileIndex = files.indexOf(d.file); // Find the index of the file
                                    const latestCommitDate = allFilesHistory[d.file]?.commitDate?.slice(-1)[0] || "N/A"; // Get the latest commit date

                                    tooltip.html(`
                                    <strong>File:</strong> ${d.file}<br>
                                    <strong>Lines of Code:</strong> ${linesOfCode[fileIndex]}<br>
                                    <strong>Changes:</strong> ${changes[fileIndex]}<br>
                                    <strong>Complexity:</strong> ${complexities[fileIndex]}<br>
                                    <strong>Size:</strong> ${sizes[fileIndex]} KB<br>
                                    <strong>Last Commit Date:</strong> ${latestCommitDate}<br>
                                    <strong>Author:</strong> ${authors[fileIndex]}  <!-- Aqui está a alteração -->
                                `)
                                        .style("display", "block")
                                        .style("left", `${event.pageX + 10}px`)
                                        .style("top", `${event.pageY + 10}px`)
                                        .style("background", color)
                                        .style("color", "#fff");
                                }


                                function hideTooltip() {
                                    tooltip.style("display", "none");
                                }

                                // Render data1
                                svg.selectAll(".bar1")
                                    .data(data1)
                                    .enter()
                                    .append("rect")
                                    .attr("class", "bar1")
                                    .attr("x", d => x(d.file))
                                    .attr("y", d => y(d.lines))
                                    .attr("width", x.bandwidth() / 2)
                                    .attr("height", d => height - y(d.lines))
                                    .attr("fill", "steelblue")
                                    .on("mouseover", (event, d) => showTooltip(event, d, "steelblue"))
                                    .on("mousemove", event => {
                                        tooltip.style("left", `${event.pageX + 10}px`).style("top", `${event.pageY + 10}px`);
                                    })
                                    .on("mouseout", hideTooltip);

                                // Render data2
                                svg.selectAll(".bar2")
                                    .data(data2)
                                    .enter()
                                    .append("rect")
                                    .attr("class", "bar2")
                                    .attr("x", d => x(d.file) + x.bandwidth() / 2)
                                    .attr("y", d => y(d.lines))
                                    .attr("width", x.bandwidth() / 2)
                                    .attr("height", d => height - y(d.lines))
                                    .attr("fill", "orange")
                                    .on("mouseover", (event, d) => showTooltip(event, d, "orange"))
                                    .on("mousemove", event => {
                                        tooltip.style("left", `${event.pageX + 10}px`).style("top", `${event.pageY + 10}px`);
                                    })
                                    .on("mouseout", hideTooltip);
                            }
                        });
                });
        }

        createHistogram();
    });
