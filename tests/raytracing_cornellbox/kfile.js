const project = new Project('raytracing_cornellbox');

await project.addProject(findKore());

project.addFile('sources/**');
project.addKongDir('shaders');
project.setDebugDir('deployment');
project.flatten();

resolve(project);
